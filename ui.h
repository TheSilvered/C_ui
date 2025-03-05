#ifndef UI_H_
#define UI_H_

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
    UISizing_fit
} UISizing;

typedef struct UIPadding {
    float top, bottom, left, right;
} UIPadding;

typedef struct UIContext UIContext;
typedef struct UIElement UIElement;

typedef struct UI__Children {
    UIElement **data;
    uint16_t len;
    uint16_t cap;
} UI__Children;

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
    uint8_t direction : 2;
    uint8_t alignX : 2;
    uint8_t alignY : 2;
    uint8_t w_sizing;
    uint8_t h_sizing;
    float childGap;
    float w_weight;
    float w_min, w_max;
    float h_weight;
    float h_min, h_max;
} UILayout;

struct UIElement {
    UIRect box;
    UILayout layout;
    UIColor backgroundColor;
    UIElement *parent;
    UIContext *context;
    UI__Children children;
};

typedef struct UI__ElementBucket {
    UIElement element;
    struct UI__ElementBucket *next;
} UI__ElementBucket;

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
    UI__ElementBucket *freeElement;
    size_t freeElementCount;
    size_t maxFreeElementCount;
};

UIElement *UIElement_New(UIElement *parent);

bool UI__Element_AddChild(UIElement *parent, UIElement *child);
void UI__Element_RemoveChild(UIElement *child);

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

bool UIContext_Init(UIContext *ctx, size_t maxElementCount, void *userData);

UIElement *UI__Context_AllocElement(UIContext *ctx);
void UI__Context_FreeElement(UIContext *ctx, UIElement *element);

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

float UI_fmax2(float a, float b);
float UI_fmax3(float a, float b, float c);

// Implementation specific functions

void *UI_MemAlloc(size_t size);
void *UI_MemExpand(void *block, size_t size);
void *UI_MemShrink(void *block, size_t size);
void UI_MemFree(void *block);

bool UI_DrawRect(UIContext *ctx, UIRect rect, UIColor color);

#ifdef UI_IMPLEMENTATION

UIElement *UIElement_New(UIElement *parent) {
    UIElement *element = UI__Context_AllocElement(parent->context);
    if (element == NULL)
        return NULL;
    if (!UI__Element_AddChild(parent, element))
        return NULL;
    return element;
}

bool UI__Element_AddChild(UIElement *parent, UIElement *child) {
    if (parent == NULL || child == NULL)
        return false;
    if (child->parent != NULL)
        return false;

    child->parent = parent;

    if (parent->children.len < parent->children.cap) {
        parent->children.data[parent->children.len++] = child;
        return true;
    }
    UIElement **newData;
    if (parent->children.data == NULL)
        newData = (UIElement **)UI_MemAlloc(sizeof(UIElement *) * 2);
    else {
        newData = (UIElement **)UI_MemExpand(
            parent->children.data,
            sizeof(UIElement *) * parent->children.cap * 2);
    }

    if (newData == NULL)
        return false;
    if (parent->children.cap == 0)
        parent->children.cap = 2;
    else
        parent->children.cap *= 2;
    parent->children.data = newData;
    newData[parent->children.len++] = child;
    return true;
}

void UI__Element_RemoveChild(UIElement *child) {
    UIElement *parent = child->parent;
    if (parent == NULL)
        return;
    child->parent = NULL;
    for (size_t i = 0, n = parent->children.len; i < n; i++) {
        UIElement *ith_child = parent->children.data[i];
        if (ith_child == child) {
            for (size_t j = i + 1; j < n; j++)
                parent->children.data[j - 1] = parent->children.data[j];
            parent->children.len--;
            break;
        }
    }
}

void UI_BackgroundColor(UIElement *element, UIColor color) {
    element->backgroundColor = color;
}

void UI_FitWidth(UIElement *element) {
    element->layout.w_sizing = UISizing_fit;
    element->layout.w_weight = 1.0f;
}

void UI_FitHeight(UIElement *element) {
    element->layout.h_sizing = UISizing_fit;
    element->layout.w_weight = 1.0f;
}

void UI_FixedWidth(UIElement *element, float width) {
    element->layout.w_sizing = UISizing_fixed;
    element->layout.w_weight = 1.0f;
    element->layout.w_min = width;
    element->layout.w_max = width;
}

void UI_FixedHeight(UIElement *element, float height) {
    element->layout.h_sizing = UISizing_fixed;
    element->layout.h_weight = 0.0f;
    element->layout.h_min = height;
    element->layout.h_max = height;
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
    ctx->freeElement = NULL;
    ctx->maxFreeElementCount = maxElementCount;
    ctx->freeElementCount = 0;
    UIElement *root = UI__Context_AllocElement(ctx);
    if (!root)
        return false;
    UI_FixedWidth(root, 0);
    UI_FixedHeight(root, 0);

    ctx->root = root;
    ctx->window.w = 0;
    ctx->window.h = 0;

    return true;
}

UIElement *UI__Context_AllocElement(UIContext *ctx) {
    UIElement *element = NULL;
    if (ctx->freeElement != NULL) {
        element = &ctx->freeElement->element;
        ctx->freeElement = ctx->freeElement->next;
        ctx->freeElementCount--;
    } else {
        UI__ElementBucket *bucket = (UI__ElementBucket *)UI_MemAlloc(sizeof(UI__ElementBucket));
        if (bucket == NULL)
            return NULL;
        element = &bucket->element;
    }

    element->context = ctx;
    element->box = (UIRect) { 0, 0, 0, 0 };
    element->parent = NULL;
    element->backgroundColor = (UIColor) { 255, 255, 255, 255 };
    element->children = (UI__Children) { .len = 0, .cap = 0, .data = NULL };
    element->layout = (UILayout) {
        .padding = { 0, 0, 0, 0 },
        .margin = { 0, 0, 0, 0 },
        .childGap = 0,
        .alignX = UIAlignX_left,
        .alignY = UIAlignY_top,
        .direction = UILayoutDirection_topToBottom,
        .w_sizing = UISizing_fit,
        .w_min = 0.0f,
        .w_max = 0.0f,
        .w_weight = 1.0f,
        .h_sizing = UISizing_fit,
        .h_min = 0.0f,
        .h_max = 0.0f,
        .h_weight = 1.0f
    };
    return element;
}

void UI__Context_FreeElement(UIContext *ctx, UIElement *element) {
    if (ctx->freeElementCount < ctx->maxFreeElementCount) {
        UI__ElementBucket *bucket = (UI__ElementBucket *)element;
        bucket->next = ctx->freeElement;
        ctx->freeElement = bucket;
        ctx->freeElementCount++;
    } else {
        UI_MemFree(element);
    }
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
    for (size_t i = 0, n = element->children.len; i < n; i++)
        UI__ElementFitSize(element->children.data[i]);

    switch (element->layout.w_sizing) {
    case UISizing_fixed:
        element->box.w = element->layout.w_min;
        break;
    case UISizing_fit:
        UI__ElementFitWidth(element);
    default:
        break;
    }

    switch (element->layout.h_sizing) {
    case UISizing_fixed:
        element->box.h = element->layout.h_min;
        break;
    case UISizing_fit:
        UI__ElementFitHeight(element);
    default:
        break;
    }
}

void UI__ElementFitWidth(UIElement *element) {
    UIPadding padding = element->layout.padding;
    if (element->children.len == 0) {
        element->box.w = padding.left + padding.right;
        return;
    }

    float w = 0;

    bool sumWidth = element->layout.direction == UILayoutDirection_leftToRight
                 || element->layout.direction == UILayoutDirection_rightToLeft;

    float prevMargin = padding.left;
    for (size_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        UIPadding margin = child->layout.margin;

        if (sumWidth) {
            float gap = i == 0 ? 0 : element->layout.childGap;
            w += child->box.w + UI_fmax3(prevMargin, margin.left, gap);
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
    if (element->children.len == 0) {
        element->box.h = padding.top + padding.bottom;
        return;
    }

    float h = 0;

    bool sumHeight = element->layout.direction == UILayoutDirection_topToBottom
                  || element->layout.direction == UILayoutDirection_bottomToTop;

    float prevMargin = padding.top;
    for (size_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        UIPadding margin = child->layout.margin;

        if (sumHeight) {
            float gap = i == 0 ? 0 : element->layout.childGap;
            h += child->box.h + UI_fmax3(prevMargin, margin.top, gap);
            prevMargin = margin.bottom;
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
    UI__ElementGrowWidth(element);
    UI__ElementGrowHeight(element);

    for (size_t i = 0, n = element->children.len; i < n; i++)
        UI__ElementGrowSize(element->children.data[i]);
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

    for (size_t i = 0, n = element->children.len; i < n; i++)
        UI__ElementPosition(element->children.data[i]);
}

void UI__ElementPositionX(UIElement *element) {
    float baseX = element->box.x;
    float elementW = element->box.w;
    UIPadding padding = element->layout.padding;

    if (element->layout.direction == UILayoutDirection_topToBottom ||
        element->layout.direction == UILayoutDirection_bottomToTop)
    {
        for (size_t i = 0, n = element->children.len; i < n; i++) {
            UIElement *child = element->children.data[i];
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

    for (size_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[reverseChildren ? n - i - 1 : i];
        float gap = i == 0 ? 0 : element->layout.childGap;
        childWidth += UI_fmax3(prevMargin, child->layout.margin.left, gap);
        child->box.x = childWidth;
        childWidth += child->box.w;
        prevMargin = child->layout.margin.right;
    }
    childWidth += UI_fmax2(prevMargin, padding.right);
    float childOffset = 0;
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

    for (size_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
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
        for (size_t i = 0, n = element->children.len; i < n; i++) {
            UIElement *child = element->children.data[i];
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

    for (size_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[reverseChildren ? n - i - 1 : i];
        float gap = i == 0 ? 0 : element->layout.childGap;
        childHeight += UI_fmax3(prevMargin, child->layout.margin.top, gap);
        child->box.y = childHeight;
        childHeight += child->box.h;
        prevMargin = child->layout.margin.bottom;
    }
    childHeight += UI_fmax2(prevMargin, padding.bottom);
    float childOffset = 0;
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

    for (size_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        child->box.y += baseY + childOffset;
    }
}

bool UI__ElementDraw(UIElement *element) {
    if (!UI_DrawRect(element->context, element->box, element->backgroundColor))
        return false;
    for (size_t i = 0, n = element->children.len; i < n; i++) {
        if (!UI__ElementDraw(element->children.data[i]))
            return false;
    }
    return true;
}

float UI_fmax2(float a, float b) {
    return a > b ? a : b;
}

float UI_fmax3(float a, float b, float c) {
    return a > b && a > c ? a : b > c ? b : c;
}

#endif // !UI_IMPLEMENTATION

#endif // !UI_H_
