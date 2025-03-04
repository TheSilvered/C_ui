#ifndef UI_H_
#define UI_H_

#define UI_IMPLEMENTATION

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define UI_DEFAULT_MAX_ELEMENT_COUNT 8192

typedef struct UIRect {
    float x, y, w, h;
} UIRect;

typedef struct UIColor {
    uint8_t r, g, b, a;
} UIColor;

typedef enum UISizing {
    UISizing_fixed,
    UISizing_fill,
    UISizing_fit,
    UISizing_percent
} UISizing;

typedef struct UISizeAxis {
    float percent;
    float min, max;
    UISizing type;
} UISizeAxis;

typedef struct UIPadding {
    float top, bottom, left, right;
} UIPadding;

typedef struct UIContext UIContext;
typedef struct UIElement UIElement;

typedef struct UIChildren {
    UIElement **data;
    size_t len;
    size_t cap;
} UIChildren;

typedef struct UIString {
    char *text;
    size_t len;
} UIString;

typedef enum UILayoutDirection {
    UILayoutDirection_topToBottom,
    UILayoutDirection_bottomToTop,
    UILayoutDirection_leftToRight,
    UILayoutDirection_rightToLeft,
} UILayoutDirection;

typedef enum UIAlignX {
    UIAlignX_left,
    UIAlignX_right,
    UIAlignX_center
} UIAlignX;

typedef enum UIAlignY {
    UIAlignY_top,
    UIAlignY_bottom,
    UIAlignY_center
} UIAlignY;

typedef struct UILayout {
    UIPadding padding;
    UIPadding margin;
    float childGap;
    UILayoutDirection direction;
    UIAlignX alignX;
    UIAlignY alignY;
    UISizeAxis w, h;
} UILayout;

struct UIElement {
    UIRect box;
    UILayout layout;
    UIColor backgroundColor;
    UIElement *parent;
    UIContext *context;
    union {
        UIChildren children;
        UIString text;
    } content;
};

typedef struct UIElementAllocator {
    size_t capacity;
    size_t freeBucketCount;
    size_t nextFreeBucket;
    size_t *freeIndices;
    UIElement *data;
} UIElementAllocator;

typedef struct UIMouse {
    int32_t x, y;
    uint8_t buttons;
} UIMouse;

typedef struct UIWindow {
    uint32_t w, h;
} UIWindow;

struct UIContext {
    void *userData;
    UIWindow window;
    UIElement *root;
    UIElementAllocator elements;
};

bool UI__ElementAllocator_Init(UIElementAllocator *alloc, size_t capacity);
void UI__ElementAllocator_Destroy(UIElementAllocator *alloc);
UIElement *UI__ElementAllocator_Alloc(UIElementAllocator *alloc);
void UI__ElementAllocator_Free(UIElementAllocator *alloc, UIElement *element);

bool UIContext_Init(UIContext *ctx, size_t maxElementCount, void *userData);

bool UI__Element_AddChild(UIElement *parent, UIElement *child);
void UI__Element_RemoveChild(UIElement *child);

UIElement *UIElement_New(UIElement *parent);

void UI_BackgroundColor(UIElement *element, UIColor color);

void UI_FitWidth(UIElement *element);
void UI_FitHeight(UIElement *element);
void UI_FixedWidth(UIElement *element, float width);
void UI_FixedHeight(UIElement *element, float height);

void UI_Padding(UIElement *element, float padding);
void UI_PaddingEx(UIElement *element, float top, float bottom, float left, float right);
void UI_Margin(UIElement *element, float margin);
void UI_MarginEx(UIElement *element, float top, float bottom, float left, float right);
void UI_ChildGap(UIElement *element, float childGap);

void UI_AlignX(UIElement *element, UIAlignX align);
void UI_AlignY(UIElement *element, UIAlignY align);

void UI_LayoutDirection(UIElement *element, UILayoutDirection direction);

void UIContext_UpdateWindow(UIContext *ctx, uint32_t width, uint32_t height);
bool UIContext_Draw(UIContext *ctx);

void UI__ElementFitSize(UIElement *element);
void UI__ElementFitWidth(UIElement *element);
void UI__ElementFitHeight(UIElement *element);

void UI__ElementGrowSize(UIElement *element);
void UI__ElementGrowWidth(UIElement *element);
void UI__ElementGrowHeight(UIElement *element);

void UI__ElementPosition(UIElement *element);
void UI__ElementPositionX(UIElement *element);
void UI__ElementPositionY(UIElement *element);

bool UI__ElementDraw(UIElement *element);

UIElement *UI__Context_AllocElement(UIContext *ctx);
void UI__Context_FreeElement(UIContext *ctx, UIElement *element);

void UI__Context_UpdateLayout(UIContext *ctx);

float UI_fmax2(float a, float b);
float UI_fmax3(float a, float b, float c);

// Implementation specific functions

void *UI_MemAlloc(size_t size);
void *UI_MemExpand(void *block, size_t size);
void *UI_MemShrink(void *block, size_t size);
void UI_MemFree(void *block);

bool UI_DrawRect(UIContext *ctx, UIRect rect, UIColor color);

#ifdef UI_IMPLEMENTATION

bool UI__ElementAllocator_Init(UIElementAllocator *alloc, size_t capacity) {
    alloc->capacity = capacity;
    alloc->freeBucketCount = 0;
    alloc->nextFreeBucket = 0;
    alloc->freeIndices = NULL;
    alloc->data = NULL;

    size_t totalSize = sizeof(UIElement) * capacity + sizeof(size_t) * capacity;
    UIElement *data = (UIElement *)UI_MemAlloc(totalSize);
    if (data == NULL)
        return false;
    size_t *freeIndices = (size_t *)(data + capacity);

    for (size_t i = 0; i < capacity; i++)
        freeIndices[i] = i;

    alloc->data = data;
    alloc->freeIndices = freeIndices;
    alloc->freeBucketCount = capacity;

    return true;
}

void UI__ElementAllocator_Destroy(UIElementAllocator *alloc) {
    if (alloc->data != NULL)
        UI_MemFree(alloc->data);
}

UIElement *UI__ElementAllocator_Alloc(UIElementAllocator *alloc) {
    if (alloc->freeBucketCount == 0)
        return NULL;
    size_t nextFreeBucket = alloc->nextFreeBucket;
    UIElement *bucket = alloc->data + nextFreeBucket;
    alloc->freeBucketCount--;
    alloc->nextFreeBucket++;
    // Avoid %
    if (nextFreeBucket + 1 == alloc->capacity)
        alloc->nextFreeBucket = 0;
    else
        alloc->nextFreeBucket++;
    return bucket;
}

void UI__ElementAllocator_Free(UIElementAllocator *alloc, UIElement *element) {
    if (element == NULL)
        return;
    size_t bucketIdx = (element - alloc->data) / sizeof(UIElement);

    // alloc->nextFreeBucket + alloc->freeBucketCount % alloc->capacity could overflow
    size_t idxCountBeforeOverflow = alloc->capacity - alloc->nextFreeBucket;
    size_t newFreeIdx;
    if (alloc->freeBucketCount < idxCountBeforeOverflow)
        newFreeIdx = alloc->nextFreeBucket + alloc->freeBucketCount;
    else
        newFreeIdx = alloc->freeBucketCount - idxCountBeforeOverflow;

    alloc->freeIndices[newFreeIdx] = bucketIdx;
}

bool UI__Element_AddChild(UIElement *parent, UIElement *child) {
    if (parent == NULL || child == NULL)
        return false;
    if (child->parent != NULL)
        return false;

    child->parent = parent;

    if (parent->content.children.len < parent->content.children.cap) {
        parent->content.children.data[parent->content.children.len++] = child;
        return true;
    }
    UIElement **newData;
    if (parent->content.children.data == NULL)
        newData = (UIElement **)UI_MemAlloc(sizeof(UIElement *) * 2);
    else {
        newData = (UIElement **)UI_MemExpand(
            parent->content.children.data,
            sizeof(UIElement *) * parent->content.children.cap * 2);
    }

    if (newData == NULL)
        return false;
    if (parent->content.children.cap == 0)
        parent->content.children.cap = 2;
    else
        parent->content.children.cap *= 2;
    parent->content.children.data = newData;
    newData[parent->content.children.len++] = child;
    return true;
}

void UI__Element_RemoveChild(UIElement *child) {
    UIElement *parent = child->parent;
    if (parent == NULL)
        return;
    child->parent = NULL;
    for (size_t i = 0, n = parent->content.children.len; i < n; i++) {
        UIElement *ith_child = parent->content.children.data[i];
        if (ith_child == child) {
            for (size_t j = i + 1; j < n; j++)
                parent->content.children.data[j - 1] = parent->content.children.data[j];
            parent->content.children.len--;
            break;
        }
    }
}

UIElement *UIElement_New(UIElement *parent) {
    UIElement *element = UI__Context_AllocElement(parent->context);
    if (element == NULL)
        return NULL;
    if (!UI__Element_AddChild(parent, element))
        return NULL;
    return element;
}

void UI_BackgroundColor(UIElement *element, UIColor color) {
    element->backgroundColor = color;
}

void UI_FitWidth(UIElement *element) {
    element->layout.w.type = UISizing_fit;
    element->layout.w.percent = 0.0f;
}

void UI_FitHeight(UIElement *element) {
    element->layout.h.type = UISizing_fit;
    element->layout.h.percent = 0.0f;
}

void UI_FixedWidth(UIElement *element, float width) {
    element->layout.w = (UISizeAxis) {
        .type = UISizing_fixed,
        .percent = 0.0f,
        .min = width,
        .max = width
    };
}

void UI_FixedHeight(UIElement *element, float height) {
    element->layout.h = (UISizeAxis) {
        .type = UISizing_fixed,
        .percent = 0.0f,
        .min = height,
        .max = height
    };
}

void UI_Padding(UIElement *element, float padding) {
    element->layout.padding = (UIPadding) { padding, padding, padding, padding };
}

void UI_PaddingEx(UIElement *element, float top, float bottom, float left, float right) {
    element->layout.padding = (UIPadding) { top, bottom, left, right };
}

void UI_Margin(UIElement *element, float margin) {
    element->layout.margin = (UIPadding) { margin, margin, margin, margin };
}

void UI_MarginEx(UIElement *element, float top, float bottom, float left, float right) {
    element->layout.margin = (UIPadding) { top, bottom, left, right };
}

void UI_ChildGap(UIElement *element, float childGap) {
    element->layout.childGap = childGap;
}

void UI_AlignX(UIElement *element, UIAlignX align) {
    element->layout.alignX = align;
}

void UI_AlignY(UIElement *element, UIAlignY align) {
    element->layout.alignY = align;
}

void UI_LayoutDirection(UIElement *element, UILayoutDirection direction) {
    element->layout.direction = direction;
}

bool UIContext_Init(UIContext *ctx, size_t maxElementCount, void *userData) {
    ctx->userData = userData;
    if (!UI__ElementAllocator_Init(&ctx->elements, maxElementCount))
        return false;

    UIElement *root = UI__Context_AllocElement(ctx);
    if (!root) {
        UI__ElementAllocator_Destroy(&ctx->elements);
        return false;
    }
    UI_FixedWidth(root, 0);
    UI_FixedHeight(root, 0);

    ctx->root = root;
    ctx->window.w = 0;
    ctx->window.h = 0;

    return true;
}

void UIContext_UpdateWindow(UIContext *ctx, uint32_t width, uint32_t height) {
    ctx->window.w = width;
    ctx->window.h = height;
}

bool UIContext_Draw(UIContext *ctx) {
    UIElement *root = ctx->root;
    UI_FixedWidth(root, (float)ctx->window.w);
    UI_FixedHeight(root, (float)ctx->window.h);

    UI__ElementFitSize(root);
    UI__ElementGrowSize(root);
    UI__ElementPosition(root);
    return UI__ElementDraw(root);
}

void UI__ElementFitSize(UIElement *element) {
    for (size_t i = 0, n = element->content.children.len; i < n; i++)
        UI__ElementFitSize(element->content.children.data[i]);

    switch (element->layout.w.type) {
    case UISizing_fixed:
        element->box.w = element->layout.w.min;
        break;
    case UISizing_fit:
        UI__ElementFitWidth(element);
    default:
        break;
    }

    switch (element->layout.h.type) {
    case UISizing_fixed:
        element->box.h = element->layout.h.min;
        break;
    case UISizing_fit:
        UI__ElementFitHeight(element);
    default:
        break;
    }
}

void UI__ElementFitWidth(UIElement *element) {
    UIPadding padding = element->layout.padding;
    if (element->content.children.len == 0) {
        element->box.w = padding.left + padding.right;
        return;
    }

    float w = 0;

    bool sumWidth = element->layout.direction == UILayoutDirection_leftToRight
                 || element->layout.direction == UILayoutDirection_rightToLeft;

    float prevMargin = padding.left;
    for (size_t i = 0, n = element->content.children.len; i < n; i++) {
        UIElement *child = element->content.children.data[i];
        UIPadding margin = child->layout.margin;

        if (sumWidth) {
            float gap = i == 0 ? 0 : element->layout.childGap;
            w += child->box.w + UI_fmax3(prevMargin, padding.left, gap);
            prevMargin = margin.right;
        } else {
            float leftSpace = UI_fmax2(padding.left, margin.left);
            float rightSpace = UI_fmax2(padding.right, margin.right);
            float totalWidth = child->box.w + leftSpace + rightSpace;
            if (w < totalWidth)
                w = totalWidth;
        }
    }

    if (sumWidth)
        w += UI_fmax2(padding.right, prevMargin);

    element->box.w = w;
}

void UI__ElementFitHeight(UIElement *element) {
    UIPadding padding = element->layout.padding;
    if (element->content.children.len == 0) {
        element->box.h = padding.top + padding.bottom;
        return;
    }

    float h = 0;

    bool sumHeight = element->layout.direction == UILayoutDirection_topToBottom
                  || element->layout.direction == UILayoutDirection_bottomToTop;

    float prevMargin = padding.top;
    for (size_t i = 0, n = element->content.children.len; i < n; i++) {
        UIElement *child = element->content.children.data[i];
        UIPadding margin = child->layout.margin;

        if (sumHeight) {
            float gap = i == 0 ? 0 : element->layout.childGap;
            h += child->box.h + UI_fmax3(prevMargin, padding.top, gap);
            prevMargin = margin.right;
        } else {
            float topSpace = UI_fmax2(padding.top, margin.top);
            float bottomSpace = UI_fmax2(padding.bottom, margin.bottom);
            float totalHeight = child->box.h + topSpace + bottomSpace;
            if (h < totalHeight)
                h = totalHeight;
        }
    }

    if (sumHeight)
        h += UI_fmax2(padding.bottom, prevMargin);

    element->box.h = h;
}

void UI__ElementGrowSize(UIElement *element) {
    (void)element;
}

void UI__ElementGrowWidth(UIElement *element) {
    (void)element;
}

void UI__ElementGrowHeight(UIElement *element) {
    (void)element;
}

void UI__ElementPosition(UIElement *element) {
    UI__ElementPositionX(element);
    UI__ElementPositionY(element);

    for (size_t i = 0, n = element->content.children.len; i < n; i++)
        UI__ElementPosition(element->content.children.data[i]);
}

void UI__ElementPositionX(UIElement *element) {
    float baseX = element->box.x;
    float elementW = element->box.w;
    UIPadding padding = element->layout.padding;

    if (element->layout.direction == UILayoutDirection_topToBottom ||
        element->layout.direction == UILayoutDirection_bottomToTop)
    {
        for (size_t i = 0, n = element->content.children.len; i < n; i++) {
            UIElement *child = element->content.children.data[i];
            switch (element->layout.alignX) {
            case UIAlignX_left:
                child->box.x = baseX + UI_fmax2(padding.left, child->layout.margin.left);
                break;
            case UIAlignX_right:
                child->box.x = baseX + elementW - child->box.w - UI_fmax2(padding.right, child->layout.margin.right);
                break;
            case UIAlignX_center: {
                float offset = (elementW - child->box.w) / 2.0f;
                float leftSpace = UI_fmax2(padding.left, child->layout.margin.left);
                float rightSpace = UI_fmax2(padding.right, child->layout.margin.right);
                printf("%f\n", offset);
                if (offset < leftSpace)
                    offset = leftSpace;
                else if (elementW - offset - child->box.w < rightSpace)
                    offset = elementW - child->box.w - rightSpace;
                child->box.x = baseX + offset;
                break;
            }
            }
        }
        return;
    }
    float childWidth = 0;
    float prevMargin = padding.left;
    bool reverseChildren = element->layout.direction == UILayoutDirection_rightToLeft;

    for (size_t i = 0, n = element->content.children.len; i < n; i++) {
        UIElement *child = element->content.children.data[reverseChildren ? n - i - 1 : i];
        float gap = i == 0 ? 0 : element->layout.childGap;
        childWidth += UI_fmax3(prevMargin, child->layout.margin.left, gap);
        child->box.x = childWidth;
        childWidth += child->box.w;
        prevMargin = child->layout.margin.right;
    }
    childWidth += UI_fmax2(prevMargin, padding.right);
    float childOffset;
    switch (element->layout.alignX) {
        case UIAlignX_left:
            childOffset = 0;
            break;
        case UIAlignX_right:
            childOffset = elementW - childWidth;
            break;
        case UIAlignX_center:
            childOffset = (elementW - childWidth) / 2;
            break;
    }

    for (size_t i = 0, n = element->content.children.len; i < n; i++) {
        UIElement *child = element->content.children.data[i];
        child->box.x += baseX + childOffset;
    }
}

void UI__ElementPositionY(UIElement *element) {
    float baseY = element->box.y;
    float elementH = element->box.h;
    UIPadding padding = element->layout.padding;

    if (element->layout.direction == UILayoutDirection_leftToRight ||
        element->layout.direction == UILayoutDirection_rightToLeft)
    {
        for (size_t i = 0, n = element->content.children.len; i < n; i++) {
            UIElement *child = element->content.children.data[i];
            switch (element->layout.alignY) {
            case UIAlignY_top:
                child->box.y = baseY + UI_fmax2(padding.top, child->layout.margin.top);
                break;
            case UIAlignY_bottom:
                child->box.y = baseY + elementH - child->box.h - UI_fmax2(padding.bottom, child->layout.margin.bottom);
                break;
            case UIAlignY_center: {
                float offset = (elementH - child->box.h) / 2.0f;
                float topSpace = UI_fmax2(padding.top, child->layout.margin.top);
                float bottomSpace = UI_fmax2(padding.bottom, child->layout.margin.bottom);
                if (offset < topSpace)
                    offset = topSpace;
                else if (elementH - offset - child->box.h < bottomSpace)
                    offset = elementH - child->box.h - bottomSpace;
                child->box.y = baseY + offset;
                break;
            }
            }
        }
        return;
    }
    float childHeight = 0;
    float prevMargin = padding.top;
    bool reverseChildren = element->layout.direction == UILayoutDirection_rightToLeft;

    for (size_t i = 0, n = element->content.children.len; i < n; i++) {
        UIElement *child = element->content.children.data[reverseChildren ? n - i - 1 : i];
        float gap = i == 0 ? 0 : element->layout.childGap;
        childHeight += UI_fmax3(prevMargin, child->layout.margin.top, gap);
        child->box.y = childHeight;
        childHeight += child->box.h;
        prevMargin = child->layout.margin.bottom;
    }
    childHeight += UI_fmax2(prevMargin, padding.bottom);
    float childOffset;
    switch (element->layout.alignY) {
        case UIAlignY_top:
            childOffset = 0;
            break;
        case UIAlignY_bottom:
            childOffset = elementH - childHeight;
            break;
        case UIAlignY_center:
            childOffset = (elementH - childHeight) / 2;
            break;
    }

    for (size_t i = 0, n = element->content.children.len; i < n; i++) {
        UIElement *child = element->content.children.data[i];
        child->box.y += baseY + childOffset;
    }
}

bool UI__ElementDraw(UIElement *element) {
    if (!UI_DrawRect(element->context, element->box, element->backgroundColor))
        return false;
    for (size_t i = 0, n = element->content.children.len; i < n; i++) {
        if (!UI__ElementDraw(element->content.children.data[i]))
            return false;
    }
    return true;
}

UIElement *UI__Context_AllocElement(UIContext *ctx) {
    UIElement *element = UI__ElementAllocator_Alloc(&ctx->elements);
    element->context = ctx;
    element->box = (UIRect) { 0, 0, 0, 0 };
    element->parent = NULL;
    element->backgroundColor = (UIColor) { 255, 255, 255, 255 };
    element->content.children = (UIChildren) { .len = 0, .cap = 0, .data = NULL };
    element->layout = (UILayout) {
        .padding = { 0, 0, 0, 0 },
        .margin = { 0, 0, 0, 0 },
        .childGap = 0,
        .alignX = UIAlignX_left,
        .alignY = UIAlignY_top,
        .direction = UILayoutDirection_topToBottom,
        .w = { .type = UISizing_fit, .min = 0, .max = 0 },
        .h = { .type = UISizing_fit, .min = 0, .max = 0 }
    };
    return element;
}

void UI__Context_FreeElement(UIContext *ctx, UIElement *element) {
    UI__ElementAllocator_Free(&ctx->elements, element);
}

float UI_fmax2(float a, float b) {
    return a > b ? a : b;
}

float UI_fmax3(float a, float b, float c) {
    return a > b && a > c ? a : b > c ? b : c;
}

#endif // !UI_IMPLEMENTATION

#endif // !UI_H_
