#ifndef UI_H_
#define UI_H_

#include <stdint.h>
#include <stdbool.h>

#define UI_IMPLEMENTATION

#define UI_MAX_ELEMENT_COUNT 8192

#define UI_RED (UIColor) { 255, 0, 0, 255 }
#define UI_GREEN (UIColor) { 0, 255, 0, 255 }
#define UI_BLUE (UIColor) { 0, 0, 255, 255 }
#define UI_BLACK (UIColor) { 0, 0, 0, 255 }
#define UI_WHITE (UIColor) { 255, 255, 255, 255 }

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
    UILayoutDirection direction : 3;
    UIAlignX alignX : 3;
    UIAlignY alignY : 3;
    UISizing w_sizing : 8;
    UISizing h_sizing : 8;
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

typedef struct UI__PoolBucket {
    struct UI__PoolBucket *next;
} UI__PoolBucket;

typedef struct UIPoolAllocator {
    uint32_t elementSize;
    uint32_t maxBucketCount;
    uint32_t bucketCount;
    UI__PoolBucket *firstBucket;
} UIPoolAllocator;

// Errors

typedef enum UIErrorKind {
    UIErrorKind_noError,

    UIErrorKind_outOfMemory,

    UI__ErrorKind_count
} UIErrorKind;

const char *UI_errorStr[UI__ErrorKind_count] = {
    [UIErrorKind_noError] = "no error",
    [UIErrorKind_outOfMemory] = "out of memory"
};

typedef struct UIWindow {
    uint32_t w, h;
} UIWindow;

struct UIContext {
    void *userData;
    UIWindow window;
    UIElement *root;
    UIPoolAllocator _elementAllocator;
    UI__Children _fillChildren; // Used to store children that are set to fill
    UIErrorKind errorKind;
};

// Pool allocator functions

// Initialize a pool allocator
void UIPoolAllocatorInit(UIPoolAllocator *allocator, uint32_t elemSize, uint32_t maxBuckets);
// Free all memory associated with `allocator`
void UIPoolAllocatorDestroy(UIPoolAllocator *allocator);

// Allocate an item in a pool allocator
void *UIPoolAllocatorAlloc(UIPoolAllocator *allocator);
// Free an item allocated with `UIPoolAllocatorAlloc`
void UIPoolAllocatorFree(UIPoolAllocator *allocator, void *data);

// Context management functions

// Initialize a context
bool UIContext_Init(UIContext *ctx, void *userData);

// Get the error kind
UIErrorKind UI_ErrorGetKind(UIContext *ctx);
// Get the error as a string
const char *UI_ErrorGetStr(UIContext *ctx);

// Update functions

void UIContext_UpdateWindow(UIContext *ctx, uint32_t width, uint32_t height);
bool UIContext_Draw(UIContext *ctx);

// Element management functions

UIElement *UIElement_New(UIElement *parent);

void UI_BackgroundColor(UIElement *element, UIColor color);

void UI_FitWidth(UIElement *element);
void UI_FitHeight(UIElement *element);
void UI_FixedWidth(UIElement *element, float width);
void UI_FixedHeight(UIElement *element, float height);
void UI_FillWidth(UIElement *element, float weight);
void UI_FillHeight(UIElement *element, float weight);

void UI_MinWidth(UIElement *element, float width);
void UI_MinHeight(UIElement *element, float height);
void UI_MaxWidth(UIElement *element, float width);
void UI_MaxHeight(UIElement *element, float height);

void UI_Padding(UIElement *element, float padding);
void UI_PaddingEx(UIElement *element, float top, float bottom, float left, float right);
void UI_Margin(UIElement *element, float margin);
void UI_MarginEx(UIElement *element, float top, float bottom, float left, float right);
void UI_ChildGap(UIElement *element, float childGap);

void UI_AlignX(UIElement *element, UIAlignX align);
void UI_AlignY(UIElement *element, UIAlignY align);

void UI_LayoutDirection(UIElement *element, UILayoutDirection direction);

// Utility functions

float UI_fmax2(float a, float b);
float UI_fmax3(float a, float b, float c);

// Implementation specific functions

void *UI_MemAlloc(uint32_t size);
void *UI_MemExpand(void *block, uint32_t size);
void *UI_MemShrink(void *block, uint32_t size);
void UI_MemFree(void *block);

bool UI_DrawRect(UIContext *ctx, UIRect rect, UIColor color);

#ifdef UI_IMPLEMENTATION

UIElement *UI__Context_AllocElement(UIContext *ctx);
void UI__Context_FreeElement(UIContext *ctx, UIElement *element);
void UI__ErrorSet(UIContext *ctx, UIErrorKind errorKind);

bool UI__ChildrenAppend(UI__Children *children, UIElement *child);
void UI__ChildrenRemoveSwap(UI__Children *children, uint32_t index);
void UI__ChildrenRemoveShift(UI__Children *children, uint32_t index);

bool UI__Element_AddChild(UIElement *parent, UIElement *child);
void UI__Element_RemoveChild(UIElement *child);

float UI__ElementChildWidth(UIElement *element);
float UI__ElementChildHeight(UIElement *element);
float UI__ElementChildMaxWidth(UIElement *element);
float UI__ElementChildMaxHeight(UIElement *element);

void UI__ElementFitSize(UIElement *element);
void UI__ElementFitWidth(UIElement *element);
void UI__ElementFitHeight(UIElement *element);

bool UI__ElementFillSize(UIElement *element);
bool UI__ElementFillWidth(UIElement *element);
bool UI__ElementFillHeight(UIElement *element);

void UI__ElementPosition(UIElement *element);
void UI__ElementPositionX(UIElement *element);
void UI__ElementPositionY(UIElement *element);

void UI__ElementSetW(UIElement *element, float w);
void UI__ElementSetH(UIElement *element, float h);
void UI__ElementSetX(UIElement *element, float x);
void UI__ElementSetY(UIElement *element, float y);

bool UI__ElementDraw(UIElement *element);

void UIPoolAllocatorInit(UIPoolAllocator *allocator, uint32_t elemSize, uint32_t maxBuckets) {
    allocator->bucketCount = 0;
    allocator->maxBucketCount = maxBuckets;
    allocator->elementSize = elemSize;
    allocator->firstBucket = NULL;
}

void UIPoolAllocatorDestroy(UIPoolAllocator *allocator) {
    UI__PoolBucket *bucket = allocator->firstBucket;
    while (bucket != NULL) {
        UI__PoolBucket *nextBucket = bucket->next;
        UI_MemFree(bucket);
        bucket = nextBucket;
    }
}

void *UIPoolAllocatorAlloc(UIPoolAllocator *allocator) {
    if (allocator->firstBucket != NULL) {
        UI__PoolBucket *bucket = allocator->firstBucket;
        allocator->firstBucket = bucket->next;
        bucket->next = NULL;
        return (void *)(bucket + 1);
    }

    UI__PoolBucket *newBucket = UI_MemAlloc(sizeof(UI__PoolBucket) + allocator->elementSize);
    if (newBucket == NULL)
        return NULL;

    newBucket->next = NULL;
    return (void *)(newBucket + 1);
}

void UIPoolAllocatorFree(UIPoolAllocator *allocator, void *data) {
    UI__PoolBucket *bucket = (UI__PoolBucket *)data - 1;
    if (allocator->maxBucketCount != 0 && allocator->bucketCount >= allocator->maxBucketCount) {
        UI_MemFree(bucket);
        return;
    }
    bucket->next = allocator->firstBucket;
    allocator->firstBucket = bucket;
}

bool UIContext_Init(UIContext *ctx, void *userData) {
    ctx->userData = userData;
    UIPoolAllocatorInit(&ctx->_elementAllocator, sizeof(UIElement), UI_MAX_ELEMENT_COUNT);
    UIElement *root = UI__Context_AllocElement(ctx);
    if (!root)
        return false;
    UI_FixedWidth(root, 0);
    UI_FixedHeight(root, 0);

    ctx->root = root;
    ctx->window.w = 0;
    ctx->window.h = 0;

    ctx->_fillChildren = (UI__Children){
        .data = NULL,
        .len = 0,
        .cap = 0
    };

    ctx->errorKind = UIErrorKind_noError;

    return true;
}

UIElement *UI__Context_AllocElement(UIContext *ctx) {
    UIElement *element = (UIElement *)UIPoolAllocatorAlloc(&ctx->_elementAllocator);

    if (element == NULL) {
        UI__ErrorSet(ctx, UIErrorKind_outOfMemory);
        return NULL;
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
    UIPoolAllocatorFree(&ctx->_elementAllocator, (void *)element);
}

void UI__ErrorSet(UIContext *ctx, UIErrorKind errorKind) {
    ctx->errorKind = errorKind;
}

UIErrorKind UI_ErrorGetKind(UIContext *ctx) {
    return ctx->errorKind;
}

const char *UI_ErrorGetStr(UIContext *ctx) {
    return UI_errorStr[ctx->errorKind];
}

bool UI__ChildrenAppend(UI__Children *children, UIElement *child) {
    if (children->len < children->cap) {
        children->data[children->len++] = child;
        return true;
    }
    UIElement **newData;
    if (children->data == NULL)
        newData = (UIElement **)UI_MemAlloc(sizeof(UIElement *) * 2);
    else
        newData = (UIElement **)UI_MemExpand(children->data, sizeof(UIElement *) * children->cap * 2);

    if (newData == NULL) {
        UI__ErrorSet(child->context, UIErrorKind_outOfMemory);
        return false;
    }
    if (children->cap == 0)
        children->cap = 2;
    else
        children->cap *= 2;
    children->data = newData;
    newData[children->len++] = child;
    return true;
}

void UI__ChildrenRemoveSwap(UI__Children *children, uint32_t index) {
    if (index >= children->len)
        return;
    children->data[index] = children->data[children->len - 1];
    children->len--;
}

void UI__ChildrenRemoveShift(UI__Children *children, uint32_t index) {
    if (index >= children->len)
        return;

    UIElement **data = children->data;
    for (uint32_t i = index, n = children->len; i < n - 1; i++) {
        data[i] = data[i + 1];
    }
    children->len--;
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
    UI__ElementFillSize(root);
    UI__ElementPosition(root);
    return UI__ElementDraw(root);
}

float UI__ElementChildWidth(UIElement *element) {
    UIPadding padding = element->layout.padding;
    float prevMargin = padding.left;
    float w = 0;
    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        UIPadding margin = child->layout.margin;

        float gap = i == 0 ? 0 : element->layout.childGap;
        if (child->layout.w_sizing == UISizing_fill)
            w += child->layout.w_min;
        else
            w += child->box.w;
        w += UI_fmax3(prevMargin, margin.left, gap);
        prevMargin = margin.right;
    }
    w += UI_fmax2(padding.right, prevMargin);
    return w;
}

float UI__ElementChildHeight(UIElement *element) {
    UIPadding padding = element->layout.padding;
    float prevMargin = padding.top;
    float h = 0;
    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        UIPadding margin = child->layout.margin;

        float gap = i == 0 ? 0 : element->layout.childGap;
        if (child->layout.h_sizing == UISizing_fill)
            h += child->layout.h_min;
        else
            h += child->box.h;
        h += UI_fmax3(prevMargin, margin.top, gap);
        prevMargin = margin.bottom;
    }
    h += UI_fmax2(padding.bottom, prevMargin);
    return h;
}

float UI__ElementChildMaxWidth(UIElement *element) {
    UIPadding padding = element->layout.padding;
    float maxW = 0;

    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        UIPadding margin = child->layout.margin;

        float leftSpace = UI_fmax2(padding.left, margin.left);
        float rightSpace = UI_fmax2(padding.right, margin.right);
        float totalWidth = 0;
        if (child->layout.w_sizing == UISizing_fill)
            totalWidth = child->layout.w_min + leftSpace + rightSpace;
        else
            totalWidth = child->box.w + leftSpace + rightSpace;
        if (maxW < totalWidth)
            maxW = totalWidth;
    }
    return maxW;
}

float UI__ElementChildMaxHeight(UIElement *element) {
    UIPadding padding = element->layout.padding;
    float maxH = 0;

    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        UIPadding margin = child->layout.margin;

        float topSpace = UI_fmax2(padding.top, margin.top);
        float bottomSpace = UI_fmax2(padding.bottom, margin.bottom);
        float totalHeight = 0;
        if (child->layout.w_sizing == UISizing_fill)
            totalHeight = child->layout.w_min + topSpace + bottomSpace;
        else
            totalHeight = child->box.w + topSpace + bottomSpace;
        if (maxH < totalHeight)
            maxH = totalHeight;
    }
    return maxH;
}

void UI__ElementFitSize(UIElement *element) {
    for (uint32_t i = 0, n = element->children.len; i < n; i++)
        UI__ElementFitSize(element->children.data[i]);

    switch (element->layout.w_sizing) {
    case UISizing_fixed:
        UI__ElementSetW(element, element->layout.w_min);
        break;
    case UISizing_fit:
        UI__ElementFitWidth(element);
    default:
        break;
    }

    switch (element->layout.h_sizing) {
    case UISizing_fixed:
        UI__ElementSetH(element, element->layout.h_min);
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
        UI__ElementSetW(element, padding.left + padding.right);
        return;
    }

    if (element->layout.direction == UILayoutDirection_leftToRight ||
        element->layout.direction == UILayoutDirection_rightToLeft)
    {
        UI__ElementSetW(element, UI__ElementChildWidth(element));
    } else
        UI__ElementSetW(element, UI__ElementChildMaxWidth(element));
}

void UI__ElementFitHeight(UIElement *element) {
    UIPadding padding = element->layout.padding;
    if (element->children.len == 0) {
        UI__ElementSetH(element, padding.top + padding.bottom);
        return;
    }

    if (element->layout.direction == UILayoutDirection_topToBottom ||
        element->layout.direction == UILayoutDirection_bottomToTop)
    {
        UI__ElementSetH(element, UI__ElementChildHeight(element));
    } else
        UI__ElementSetH(element, UI__ElementChildMaxHeight(element));
}

bool UI__ElementFillSize(UIElement *element) {
    if (!UI__ElementFillWidth(element))
        return false;
    if (!UI__ElementFillHeight(element))
        return false;

    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        if (!UI__ElementFillSize(element->children.data[i]))
            return false;
    }
    return true;
}

bool UI__ElementFillWidth(UIElement *element) {
    UIPadding padding = element->layout.padding;

    if (element->layout.direction == UILayoutDirection_topToBottom ||
        element->layout.direction == UILayoutDirection_bottomToTop)
    {
        for (uint32_t i = 0, n = element->children.len; i < n; i++) {
            UIElement *child = element->children.data[i];
            if (child->layout.w_sizing != UISizing_fill)
                continue;
            float spaceLeft = UI_fmax2(padding.left, child->layout.margin.left);
            float spaceRight = UI_fmax2(padding.right, child->layout.margin.right);
            UI__ElementSetW(child, element->box.w - spaceLeft - spaceRight);
        }
        return true;
    }

    float childWidth = UI__ElementChildWidth(element);
    float totalWeight = 0;
    UI__Children *fillChildren = &element->context->_fillChildren;
    fillChildren->len = 0;

    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        if (child->layout.w_sizing != UISizing_fill)
            continue;
        if (child->layout.w_weight <= 0) {
            UI__ElementSetW(child, child->layout.w_min);
            continue;
        }
        if (!UI__ChildrenAppend(fillChildren, child))
            return false;
        totalWeight += child->layout.w_weight;
        childWidth -= child->layout.w_min;
    }

    float spaceRemaining = element->box.w - childWidth;
    if (spaceRemaining < 0 || totalWeight == 0)
        return true;

    float prevSpaceRemaining = spaceRemaining;
    do {
        prevSpaceRemaining = spaceRemaining;
        // Set the size when it is below the minimum or above the maximum
        for (uint32_t i = 0, n = fillChildren->len; i < n; i++) {
            UIElement *child = fillChildren->data[i];
            float size = spaceRemaining * child->layout.w_weight / totalWeight;
            if (size >= child->layout.w_min && (size <= child->layout.w_max || child->layout.w_max == 0))
                continue;
            if (size < child->layout.w_min)
                size = child->layout.w_min;
            else if (size > child->layout.w_max)
                size = child->layout.w_max;

            totalWeight -= child->layout.w_weight;
            spaceRemaining -= size;
            UI__ElementSetW(child, size);

            // Remove the child from _fillChildren
            UI__ChildrenRemoveSwap(fillChildren, i);
            i--; n--;
        }
    } while (prevSpaceRemaining != spaceRemaining && spaceRemaining >= 0);

    if (spaceRemaining < 0)
        return true;

    // Expand the remaining children
    for (uint32_t i = 0, n = fillChildren->len; i < n; i++) {
        UIElement *child = fillChildren->data[i];
        UI__ElementSetW(child, spaceRemaining * child->layout.w_weight / totalWeight);
    }
    return true;
}

bool UI__ElementFillHeight(UIElement *element) {
    UIPadding padding = element->layout.padding;

    if (element->layout.direction == UILayoutDirection_leftToRight ||
        element->layout.direction == UILayoutDirection_rightToLeft)
    {
        for (uint32_t i = 0, n = element->children.len; i < n; i++) {
            UIElement *child = element->children.data[i];
            if (child->layout.h_sizing != UISizing_fill)
                continue;
            float spaceTop = UI_fmax2(padding.top, child->layout.margin.top);
            float spaceBottom = UI_fmax2(padding.bottom, child->layout.margin.bottom);
            UI__ElementSetH(child, element->box.h - spaceTop - spaceBottom);
        }
        return true;
    }

    float childHeight = UI__ElementChildHeight(element);
    float totalWeight = 0;
    UI__Children *fillChildren = &element->context->_fillChildren;
    fillChildren->len = 0;

    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[i];
        if (child->layout.h_sizing != UISizing_fill)
            continue;
        if (child->layout.h_weight <= 0) {
            UI__ElementSetH(child, child->layout.h_min);
            continue;
        }
        if (!UI__ChildrenAppend(fillChildren, child))
            return false;
        totalWeight += child->layout.h_weight;
        childHeight -= child->layout.h_min;
    }

    float spaceRemaining = element->box.h - childHeight;
    if (spaceRemaining < 0 || totalWeight == 0)
        return true;

    float prevSpaceRemaining = spaceRemaining;
    do {
        prevSpaceRemaining = spaceRemaining;
        // Set the size when it is below the minimum or above the maximum
        for (uint32_t i = 0, n = fillChildren->len; i < n; i++) {
            UIElement *child = fillChildren->data[i];
            float size = spaceRemaining * child->layout.h_weight / totalWeight;
            if (size >= child->layout.h_min && (size <= child->layout.h_max || child->layout.h_max == 0))
                continue;
            if (size < child->layout.h_min)
                size = child->layout.h_min;
            else if (size > child->layout.h_max)
                size = child->layout.h_max;

            totalWeight -= child->layout.h_weight;
            spaceRemaining -= size;
            UI__ElementSetH(child, size);

            // Remove the child from _fillChildren
            fillChildren->data[i] = fillChildren->data[fillChildren->len - 1];
            fillChildren->len--;
            i--; n--;
        }
    } while (prevSpaceRemaining != spaceRemaining && spaceRemaining >= 0);

    if (spaceRemaining < 0)
        return true;

    // Expand the remaining children
    for (uint32_t i = 0, n = fillChildren->len; i < n; i++) {
        UIElement *child = fillChildren->data[i];
        UI__ElementSetH(child, spaceRemaining * child->layout.h_weight / totalWeight);
    }
    return true;
}

void UI__ElementPosition(UIElement *element) {
    UI__ElementPositionX(element);
    UI__ElementPositionY(element);

    for (uint32_t i = 0, n = element->children.len; i < n; i++)
        UI__ElementPosition(element->children.data[i]);
}

void UI__ElementPositionX(UIElement *element) {
    float baseX = element->box.x;
    float elementW = element->box.w;
    UIPadding padding = element->layout.padding;

    if (element->layout.direction == UILayoutDirection_topToBottom ||
        element->layout.direction == UILayoutDirection_bottomToTop)
    {
        for (uint32_t i = 0, n = element->children.len; i < n; i++) {
            UIElement *child = element->children.data[i];
            switch (element->layout.alignX) {
            case UIAlignX_left:
                UI__ElementSetX(child, baseX + UI_fmax2(padding.left, child->layout.margin.left));
                break;
            case UIAlignX_right:
                UI__ElementSetX(
                    child,
                    baseX + elementW - child->box.w - UI_fmax2(padding.right, child->layout.margin.right));
                break;
            case UIAlignX_center: {
                float offset = (elementW - child->box.w) / 2.0f;
                float leftSpace = UI_fmax2(padding.left, child->layout.margin.left);
                float rightSpace = UI_fmax2(padding.right, child->layout.margin.right);
                if (offset < leftSpace)
                    offset = leftSpace;
                else if (elementW - offset - child->box.w < rightSpace)
                    offset = elementW - child->box.w - rightSpace;
                UI__ElementSetX(child, baseX + offset);
                break;
            }
            }
        }
        return;
    }
    if (element->children.len == 0)
        return;

    float childWidth = UI__ElementChildWidth(element);
    float childOffset = 0;
    bool reverseChildren = element->layout.direction == UILayoutDirection_rightToLeft;
    switch (element->layout.alignX) {
        case UIAlignX_left:
            childOffset = 0;
            break;
        case UIAlignX_right:
            childOffset = elementW - childWidth;
            break;
        case UIAlignX_center: {
            UIElement *firstChild = element->children.data[reverseChildren ? element->children.len - 1 : 0];
            UIElement *lastChild = element->children.data[reverseChildren ? 0 : element->children.len - 1];
            float leftSpace = UI_fmax2(padding.left, firstChild->layout.margin.left);
            float rightSpace = UI_fmax2(padding.right, lastChild->layout.margin.right);
            childWidth -= leftSpace + rightSpace;
            float offset = (elementW - childWidth) / 2;
            if (offset < leftSpace)
                offset = leftSpace;
            else if (elementW - offset - childWidth < rightSpace)
                offset = elementW - childWidth - rightSpace;
            break;
        }
    }

    float xOffset = 0;
    float prevMargin = padding.left;
    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[reverseChildren ? n - i - 1 : i];
        float gap = i == 0 ? 0 : element->layout.childGap;
        xOffset += UI_fmax3(prevMargin, child->layout.margin.left, gap);
        UI__ElementSetX(child, baseX + xOffset + childOffset);
        xOffset += child->box.w;
        prevMargin = child->layout.margin.right;
    }
}

void UI__ElementPositionY(UIElement *element) {
    float baseY = element->box.y;
    float elementH = element->box.h;
    UIPadding padding = element->layout.padding;

    if (element->layout.direction == UILayoutDirection_leftToRight ||
        element->layout.direction == UILayoutDirection_rightToLeft)
    {
        for (uint32_t i = 0, n = element->children.len; i < n; i++) {
            UIElement *child = element->children.data[i];
            switch (element->layout.alignY) {
            case UIAlignY_top:
                UI__ElementSetY(child, baseY + UI_fmax2(padding.top, child->layout.margin.top));
                break;
            case UIAlignY_bottom:
                UI__ElementSetY(
                    child,
                    baseY + elementH - child->box.h - UI_fmax2(padding.bottom, child->layout.margin.bottom));
                break;
            case UIAlignY_center: {
                float offset = (elementH - child->box.h) / 2.0f;
                float topSpace = UI_fmax2(padding.top, child->layout.margin.top);
                float bottomSpace = UI_fmax2(padding.bottom, child->layout.margin.bottom);
                if (offset < topSpace)
                    offset = topSpace;
                else if (elementH - offset - child->box.h < bottomSpace)
                    offset = elementH - child->box.h - bottomSpace;
                UI__ElementSetY(child, baseY + offset);
                break;
            }
            }
        }
        return;
    }
    if (element->children.len == 0)
        return;

    float childHeight = UI__ElementChildHeight(element);
    float childOffset = 0;
    bool reverseChildren = element->layout.direction == UILayoutDirection_bottomToTop;
    switch (element->layout.alignY) {
    case UIAlignY_top:
        childOffset = 0;
        break;
    case UIAlignY_bottom:
        childOffset = elementH - childHeight;
        break;
    case UIAlignY_center: {
        UIElement *firstChild = element->children.data[reverseChildren ? element->children.len - 1 : 0];
        UIElement *lastChild = element->children.data[reverseChildren ? 0 : element->children.len - 1];
        float topSpace = UI_fmax2(padding.top, firstChild->layout.margin.top);
        float bottomSpace = UI_fmax2(padding.bottom, lastChild->layout.margin.bottom);
        childHeight -= topSpace + bottomSpace;
        childOffset = (elementH - childHeight) / 2;
        if (childOffset < topSpace)
            childOffset = topSpace;
        else if (elementH - childOffset - childHeight < bottomSpace)
            childOffset = elementH - childHeight - bottomSpace;
    }
    }

    float yOffset = 0;
    float prevMargin = padding.top;
    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        UIElement *child = element->children.data[reverseChildren ? n - i - 1 : i];
        float gap = i == 0 ? 0 : element->layout.childGap;
        yOffset += UI_fmax3(prevMargin, child->layout.margin.top, gap);
        UI__ElementSetY(child, baseY + yOffset + childOffset);
        yOffset += child->box.h;
        prevMargin = child->layout.margin.bottom;
    }
}

void UI__ElementSetW(UIElement *element, float w) {
    if (w < element->layout.w_min)
        w = element->layout.w_min;
    else if (w > element->layout.w_max && element->layout.w_max != 0)
        w = element->layout.w_max;
    if (w < 0)
        w = 0;
    element->box.w = w;
}

void UI__ElementSetH(UIElement *element, float h) {
    if (h < element->layout.h_min)
        h = element->layout.h_min;
    else if (h > element->layout.h_max && element->layout.h_max != 0)
        h = element->layout.h_max;
    if (h < 0)
        h = 0;
    element->box.h = h;
}

void UI__ElementSetX(UIElement *element, float x) {
    element->box.x = x;
}

void UI__ElementSetY(UIElement *element, float y) {
    element->box.y = y;
}

bool UI__ElementDraw(UIElement *element) {
    if (!UI_DrawRect(element->context, element->box, element->backgroundColor))
        return false;
    for (uint32_t i = 0, n = element->children.len; i < n; i++) {
        if (!UI__ElementDraw(element->children.data[i]))
            return false;
    }
    return true;
}

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

    return UI__ChildrenAppend(&parent->children, child);
}

void UI__Element_RemoveChild(UIElement *child) {
    UIElement *parent = child->parent;
    if (parent == NULL)
        return;
    child->parent = NULL;
    for (uint32_t i = 0, n = parent->children.len; i < n; i++) {
        UIElement *ith_child = parent->children.data[i];
        if (ith_child == child) {
            UI__ChildrenRemoveShift(&parent->children, i);
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

void UI_FillWidth(UIElement *element, float weight) {
    element->layout.w_sizing = UISizing_fill;
    element->layout.w_weight = weight;
}

void UI_FillHeight(UIElement *element, float weight) {
    element->layout.h_sizing = UISizing_fill;
    element->layout.h_weight = weight;
}

void UI_MinWidth(UIElement *element, float width) {
    element->layout.w_min = width;
}

void UI_MinHeight(UIElement *element, float height) {
    element->layout.h_min = height;
}

void UI_MaxWidth(UIElement *element, float width) {
    element->layout.w_max = width;
}

void UI_MaxHeight(UIElement *element, float height) {
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

float UI_fmax2(float a, float b) {
    return a > b ? a : b;
}

float UI_fmax3(float a, float b, float c) {
    return a > b && a > c ? a : b > c ? b : c;
}

#endif // !UI_IMPLEMENTATION

#endif // !UI_H_
