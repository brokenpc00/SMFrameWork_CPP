//
//  nanovg_core.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 27..
//

#include "nanovg_core.h"
#include <cmath>
#include <string>
#include <memory>
#include <cstdlib>

#define NVG_NOTUSED(v) for (;;) { (void)(1 ? (void)0 : ( (void)(v) ) ); break; }

#define NVG_INIT_COMMANDS_SIZE 256
#define NVG_INIT_POINTS_SIZE 128
#define NVG_INIT_PATHS_SIZE 16
#define NVG_INIT_VERTS_SIZE 256

#define NVG_KAPPA90 0.5522847493f    // Length proportional to radius of a cubic bezier handle for 90deg arcs.

#define NVG_PI 3.14159265358979323846264338327f

#define NVG_COUNTOF(arr) (sizeof(arr) / sizeof(0[arr]))


enum NVGwinding {
    NVG_CCW = 1,            // Winding for solid shapes
    NVG_CW = 2,                // Winding for holes
};

enum NVGsolidity {
    NVG_SOLID = 1,            // CCW
    NVG_HOLE = 2,            // CW
};

enum NVGcommands {
    NVG_MOVETO = 0,
    NVG_LINETO = 1,
    NVG_BEZIERTO = 2,
    NVG_CLOSE = 3,
    NVG_WINDING = 4,
};

enum NVGpointFlags
{
    NVG_PT_CORNER = 0x01,
    NVG_PT_LEFT = 0x02,
    NVG_PT_BEVEL = 0x04,
    NVG_PR_INNERBEVEL = 0x08,
};


static float nvg__sqrtf(float a) { return sqrtf(a); }
//static float nvg__modf(float a, float b) { return fmodf(a, b); }
static float nvg__sinf(float a) { return sinf(a); }
static float nvg__cosf(float a) { return cosf(a); }
static float nvg__tanf(float a) { return tanf(a); }
static float nvg__atan2f(float a,float b) { return atan2f(a, b); }
static float nvg__acosf(float a) { return acosf(a); }

static int nvg__mini(int a, int b) { return a < b ? a : b; }
static int nvg__maxi(int a, int b) { return a > b ? a : b; }
static int nvg__clampi(int a, int mn, int mx) { return a < mn ? mn : (a > mx ? mx : a); }
static float nvg__minf(float a, float b) { return a < b ? a : b; }
static float nvg__maxf(float a, float b) { return a > b ? a : b; }
static float nvg__absf(float a) { return a >= 0.0f ? a : -a; }
static float nvg__signf(float a) { return a >= 0.0f ? 1.0f : -1.0f; }
static float nvg__clampf(float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }
static float nvg__cross(float dx0, float dy0, float dx1, float dy1) { return dx1*dy0 - dx0*dy1; }

static float nvg__normalize(float *x, float* y)
{
    float d = nvg__sqrtf((*x)*(*x) + (*y)*(*y));
    if (d > 1e-6f) {
        float id = 1.0f / d;
        *x *= id;
        *y *= id;
    }
    return d;
}

static int nvg__ptEquals(float x1, float y1, float x2, float y2, float tol)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx*dx + dy*dy < tol*tol;
}

static float nvg__distPtSeg(float x, float y, float px, float py, float qx, float qy)
{
    float pqx, pqy, dx, dy, d, t;
    pqx = qx-px;
    pqy = qy-py;
    dx = x-px;
    dy = y-py;
    d = pqx*pqx + pqy*pqy;
    t = pqx*dx + pqy*dy;
    if (d > 0) t /= d;
    if (t < 0) t = 0;
    else if (t > 1) t = 1;
    dx = px + t*pqx - x;
    dy = py + t*pqy - y;
    return dx*dx + dy*dy;
}

static void nvg__deletePathCache(NanoVG::NVGpathCache* c)
{
    if (c == NULL) return;
    if (c->points != NULL) free(c->points);
    if (c->paths != NULL) free(c->paths);
    if (c->verts != NULL) free(c->verts);
    free(c);
}

static NanoVG::NVGpathCache* nvg__allocPathCache(void)
{
    NanoVG::NVGpathCache* c = (NanoVG::NVGpathCache*)malloc(sizeof(NanoVG::NVGpathCache));
    if (c == NULL) goto error;
    memset(c, 0, sizeof(NanoVG::NVGpathCache));
    
    c->points = (NanoVG::NVGpoint*)malloc(sizeof(NanoVG::NVGpoint)*NVG_INIT_POINTS_SIZE);
    if (!c->points) goto error;
    c->npoints = 0;
    c->cpoints = NVG_INIT_POINTS_SIZE;
    
    c->paths = (NanoVG::NVGpath*)malloc(sizeof(NanoVG::NVGpath)*NVG_INIT_PATHS_SIZE);
    if (!c->paths) goto error;
    c->npaths = 0;
    c->cpaths = NVG_INIT_PATHS_SIZE;
    
    c->verts = (NanoVG::NVGvertex*)malloc(sizeof(NanoVG::NVGvertex)*NVG_INIT_VERTS_SIZE);
    if (!c->verts) goto error;
    c->nverts = 0;
    c->cverts = NVG_INIT_VERTS_SIZE;
    
    return c;
error:
    nvg__deletePathCache(c);
    return NULL;
}

static void nvg__setDevicePixelRatio(NanoVG::NVGcontext* ctx, float ratio)
{
    ctx->tessTol = 0.25f / ratio;
    ctx->distTol = 0.01f / ratio;
    ctx->fringeWidth = 1.0f / ratio;
    ctx->devicePxRatio = ratio;
}

static NanoVG::NVGstate* nvg__getState(NanoVG::NVGcontext* ctx)
{
    return &ctx->states[ctx->nstates-1];
}



NanoVG::NanoVG() :
_context(nullptr)
{
}

NanoVG::~NanoVG()
{
    release();
}


NanoVG* NanoVG::create() {
    auto nano = new NanoVG();
    if (nano == nullptr || !nano->init()) {
        if (nano) {
            delete nano;
            nano = nullptr;
        }
    }
    
    return nano;
}

void NanoVG::destroy(NanoVG* nano) {
    if (nano) {
        delete nano;
        nano = nullptr;
    }
}


bool NanoVG::init() {
    
    NVGparams params;
    
    memset(&params, 0, sizeof(params));
    params.edgeAntiAlias = 0;
    
    _context = (NanoVG::NVGcontext*)malloc(sizeof(NanoVG::NVGcontext));
    memset(_context, 0, sizeof(NanoVG::NVGcontext));
    
    memset(&_context->params, 0, sizeof(NVGparams));
    params.edgeAntiAlias = 0;
    
    _context->commands = (float*)malloc(sizeof(float)*NVG_INIT_COMMANDS_SIZE);
    _context->ncommands = 0;
    _context->ccommands = NVG_INIT_COMMANDS_SIZE;
    
    _context->cache = nvg__allocPathCache();
    
    nvgSave();
    nvgReset();
    
    nvg__setDevicePixelRatio(_context, 1.0f);
    
    return true;
}

void NanoVG::release() {
    if (_context == nullptr)
        return;
    
    if (_context->commands != nullptr) {
        free(_context->commands);
    }
    
    if (_context->cache != nullptr) {
        nvg__deletePathCache(_context->cache);
    }
    
    free(_context);
    _context = nullptr;
}

void NanoVG::nvgBeginFrame(float devicePixelRatio) {
    _context->nstates = 0;
    
    nvgSave();
    nvgReset();
    
    nvg__setDevicePixelRatio(_context, devicePixelRatio);
}

// State handling
void NanoVG::nvgSave() {
    if (_context->nstates >= NVG_MAX_STATES)
        return;
    
    if (_context->nstates > 0) {
        memcpy(&_context->states[_context->nstates], &_context->states[_context->nstates-1], sizeof(NVGstate));
    }
    
    _context->nstates++;
}

void NanoVG::nvgRestore() {
    if (_context->nstates <= 1)
        return;
    
    _context->nstates--;
}

void NanoVG::nvgReset() {
    NVGstate* state = nvg__getState(_context);
    memset(state, 0, sizeof(*state));
    
    state->strokeWidth = 1.0f;
    state->miterLimit = 10.0f;
    state->lineCap = NVG_BUTT;
    state->lineJoin = NVG_MITER;
    nvgTransformIdentity(state->xform);
}



void NanoVG::nvgStrokeWidth(float width) {
    NVGstate* state = nvg__getState(_context);
    state->strokeWidth = width;
}

void NanoVG::nvgMiterLimit(float limit) {
    NVGstate* state = nvg__getState(_context);
    state->miterLimit = limit;
}

void NanoVG::nvgLineCap(int cap) {
    NVGstate* state = nvg__getState(_context);
    state->lineCap = cap;
}

void NanoVG::nvgLineJoin(int join) {
    NVGstate* state = nvg__getState(_context);
    state->lineJoin = join;
}







void NanoVG::nvgTransformIdentity(float* t)
{
    t[0] = 1.0f; t[1] = 0.0f;
    t[2] = 0.0f; t[3] = 1.0f;
    t[4] = 0.0f; t[5] = 0.0f;
}

void NanoVG::nvgTransformPoint(float* dx, float* dy, const float* t, float sx, float sy)
{
    *dx = sx*t[0] + sy*t[2] + t[4];
    *dy = sx*t[1] + sy*t[3] + t[5];
}

static void nvg__appendCommands(NanoVG::NVGcontext* ctx, float* vals, int nvals)
{
    NanoVG::NVGstate* state = nvg__getState(ctx);
    int i;
    
    if (ctx->ncommands+nvals > ctx->ccommands) {
        float* commands;
        int ccommands = ctx->ncommands+nvals + ctx->ccommands/2;
        commands = (float*)realloc(ctx->commands, sizeof(float)*ccommands);
        if (commands == NULL) return;
        ctx->commands = commands;
        ctx->ccommands = ccommands;
    }
    
    if ((int)vals[0] != NVG_CLOSE && (int)vals[0] != NVG_WINDING) {
        ctx->commandx = vals[nvals-2];
        ctx->commandy = vals[nvals-1];
    }
    
    // transform commands
    i = 0;
    while (i < nvals) {
        int cmd = (int)vals[i];
        switch (cmd) {
            case NVG_MOVETO:
                NanoVG::nvgTransformPoint(&vals[i+1],&vals[i+2], state->xform, vals[i+1],vals[i+2]);
                i += 3;
                break;
            case NVG_LINETO:
                NanoVG::nvgTransformPoint(&vals[i+1],&vals[i+2], state->xform, vals[i+1],vals[i+2]);
                i += 3;
                break;
            case NVG_BEZIERTO:
                NanoVG::nvgTransformPoint(&vals[i+1],&vals[i+2], state->xform, vals[i+1],vals[i+2]);
                NanoVG::nvgTransformPoint(&vals[i+3],&vals[i+4], state->xform, vals[i+3],vals[i+4]);
                NanoVG::nvgTransformPoint(&vals[i+5],&vals[i+6], state->xform, vals[i+5],vals[i+6]);
                i += 7;
                break;
            case NVG_CLOSE:
                i++;
                break;
            case NVG_WINDING:
                i += 2;
                break;
            default:
                i++;
        }
    }
    
    memcpy(&ctx->commands[ctx->ncommands], vals, nvals*sizeof(float));
    
    ctx->ncommands += nvals;
}


static void nvg__clearPathCache(NanoVG::NVGcontext* ctx)
{
    ctx->cache->npoints = 0;
    ctx->cache->npaths = 0;
}

static NanoVG::NVGpath* nvg__lastPath(NanoVG::NVGcontext* ctx)
{
    if (ctx->cache->npaths > 0)
        return &ctx->cache->paths[ctx->cache->npaths-1];
    
    return NULL;
}

static void nvg__addPath(NanoVG::NVGcontext* ctx)
{
    NanoVG::NVGpath* path;
    if (ctx->cache->npaths+1 > ctx->cache->cpaths) {
        NanoVG::NVGpath* paths;
        int cpaths = ctx->cache->npaths+1 + ctx->cache->cpaths/2;
        paths = (NanoVG::NVGpath*)realloc(ctx->cache->paths, sizeof(NanoVG::NVGpath)*cpaths);
        if (paths == NULL) return;
        ctx->cache->paths = paths;
        ctx->cache->cpaths = cpaths;
    }
    path = &ctx->cache->paths[ctx->cache->npaths];
    memset(path, 0, sizeof(*path));
    path->first = ctx->cache->npoints;
    path->winding = NVG_CCW;
    
    ctx->cache->npaths++;
}

static NanoVG::NVGpoint* nvg__lastPoint(NanoVG::NVGcontext* ctx)
{
    if (ctx->cache->npoints > 0)
        return &ctx->cache->points[ctx->cache->npoints-1];
    return NULL;
}

static void nvg__addPoint(NanoVG::NVGcontext* ctx, float x, float y, int flags)
{
    NanoVG::NVGpath* path = nvg__lastPath(ctx);
    NanoVG::NVGpoint* pt;
    if (path == NULL) return;
    
    if (ctx->cache->npoints > 0) {
        pt = nvg__lastPoint(ctx);
        if (nvg__ptEquals(pt->x,pt->y, x,y, ctx->distTol)) {
            pt->flags |= flags;
            return;
        }
    }
    
    if (ctx->cache->npoints+1 > ctx->cache->cpoints) {
        NanoVG::NVGpoint* points;
        int cpoints = ctx->cache->npoints+1 + ctx->cache->cpoints/2;
        points = (NanoVG::NVGpoint*)realloc(ctx->cache->points, sizeof(NanoVG::NVGpoint)*cpoints);
        if (points == NULL) return;
        ctx->cache->points = points;
        ctx->cache->cpoints = cpoints;
    }
    
    pt = &ctx->cache->points[ctx->cache->npoints];
    memset(pt, 0, sizeof(*pt));
    pt->x = x;
    pt->y = y;
    pt->flags = (unsigned char)flags;
    
    ctx->cache->npoints++;
    path->count++;
}

static void nvg__closePath(NanoVG::NVGcontext* ctx)
{
    NanoVG::NVGpath* path = nvg__lastPath(ctx);
    if (path == NULL) return;
    path->closed = 1;
}

static void nvg__pathWinding(NanoVG::NVGcontext* ctx, int winding)
{
    NanoVG::NVGpath* path = nvg__lastPath(ctx);
    if (path == NULL) return;
    path->winding = winding;
}

static float nvg__getAverageScale(float *t)
{
    float sx = sqrtf(t[0]*t[0] + t[2]*t[2]);
    float sy = sqrtf(t[1]*t[1] + t[3]*t[3]);
    return (sx + sy) * 0.5f;
}

static NanoVG::NVGvertex* nvg__allocTempVerts(NanoVG::NVGcontext* ctx, int nverts)
{
    if (nverts > ctx->cache->cverts) {
        NanoVG::NVGvertex* verts;
        int cverts = (nverts + 0xff) & ~0xff; // Round up to prevent allocations when things change just slightly.
        verts = (NanoVG::NVGvertex*)realloc(ctx->cache->verts, sizeof(NanoVG::NVGvertex)*cverts);
        if (verts == NULL) return NULL;
        ctx->cache->verts = verts;
        ctx->cache->cverts = cverts;
    }
    
    return ctx->cache->verts;
}

static float nvg__triarea2(float ax, float ay, float bx, float by, float cx, float cy)
{
    float abx = bx - ax;
    float aby = by - ay;
    float acx = cx - ax;
    float acy = cy - ay;
    return acx*aby - abx*acy;
}

static float nvg__polyArea(NanoVG::NVGpoint* pts, int npts)
{
    int i;
    float area = 0;
    for (i = 2; i < npts; i++) {
        NanoVG::NVGpoint* a = &pts[0];
        NanoVG::NVGpoint* b = &pts[i-1];
        NanoVG::NVGpoint* c = &pts[i];
        area += nvg__triarea2(a->x,a->y, b->x,b->y, c->x,c->y);
    }
    return area * 0.5f;
}

static void nvg__polyReverse(NanoVG::NVGpoint* pts, int npts)
{
    NanoVG::NVGpoint tmp;
    int i = 0, j = npts-1;
    while (i < j) {
        tmp = pts[i];
        pts[i] = pts[j];
        pts[j] = tmp;
        i++;
        j--;
    }
}

static void nvg__vset(NanoVG::NVGvertex* vtx, float x, float y, float u, float v)
{
    vtx->x = x;
    vtx->y = y;
    vtx->u = u;
    vtx->v = v;
}

static void nvg__tesselateBezier(NanoVG::NVGcontext* ctx,
                                 float x1, float y1, float x2, float y2,
                                 float x3, float y3, float x4, float y4,
                                 int level, int type)
{
    float x12,y12,x23,y23,x34,y34,x123,y123,x234,y234,x1234,y1234;
    float dx,dy,d2,d3;
    
    if (level > 10) return;
    
    x12 = (x1+x2)*0.5f;
    y12 = (y1+y2)*0.5f;
    x23 = (x2+x3)*0.5f;
    y23 = (y2+y3)*0.5f;
    x34 = (x3+x4)*0.5f;
    y34 = (y3+y4)*0.5f;
    x123 = (x12+x23)*0.5f;
    y123 = (y12+y23)*0.5f;
    
    dx = x4 - x1;
    dy = y4 - y1;
    d2 = nvg__absf(((x2 - x4) * dy - (y2 - y4) * dx));
    d3 = nvg__absf(((x3 - x4) * dy - (y3 - y4) * dx));
    
    if ((d2 + d3)*(d2 + d3) < ctx->tessTol * (dx*dx + dy*dy)) {
        nvg__addPoint(ctx, x4, y4, type);
        return;
    }
    
    /*    if (nvg__absf(x1+x3-x2-x2) + nvg__absf(y1+y3-y2-y2) + nvg__absf(x2+x4-x3-x3) + nvg__absf(y2+y4-y3-y3) < ctx->tessTol) {
     nvg__addPoint(ctx, x4, y4, type);
     return;
     }*/
    
    x234 = (x23+x34)*0.5f;
    y234 = (y23+y34)*0.5f;
    x1234 = (x123+x234)*0.5f;
    y1234 = (y123+y234)*0.5f;
    
    nvg__tesselateBezier(ctx, x1,y1, x12,y12, x123,y123, x1234,y1234, level+1, 0);
    nvg__tesselateBezier(ctx, x1234,y1234, x234,y234, x34,y34, x4,y4, level+1, type);
}

static void nvg__flattenPaths(NanoVG::NVGcontext* ctx)
{
    NanoVG::NVGpathCache* cache = ctx->cache;
    NanoVG::NVGpoint* last;
    NanoVG::NVGpoint* p0;
    NanoVG::NVGpoint* p1;
    NanoVG::NVGpoint* pts;
    NanoVG::NVGpath* path;
    int i, j;
    float* cp1;
    float* cp2;
    float* p;
    float area;
    
    if (cache->npaths > 0)
        return;
    
    // Flatten
    i = 0;
    while (i < ctx->ncommands) {
        int cmd = (int)ctx->commands[i];
        switch (cmd) {
            case NVG_MOVETO:
                nvg__addPath(ctx);
                p = &ctx->commands[i+1];
                nvg__addPoint(ctx, p[0], p[1], NVG_PT_CORNER);
                i += 3;
                break;
            case NVG_LINETO:
                p = &ctx->commands[i+1];
                nvg__addPoint(ctx, p[0], p[1], NVG_PT_CORNER);
                i += 3;
                break;
            case NVG_BEZIERTO:
                last = nvg__lastPoint(ctx);
                if (last != NULL) {
                    cp1 = &ctx->commands[i+1];
                    cp2 = &ctx->commands[i+3];
                    p = &ctx->commands[i+5];
                    nvg__tesselateBezier(ctx, last->x,last->y, cp1[0],cp1[1], cp2[0],cp2[1], p[0],p[1], 0, NVG_PT_CORNER);
                }
                i += 7;
                break;
            case NVG_CLOSE:
                nvg__closePath(ctx);
                i++;
                break;
            case NVG_WINDING:
                nvg__pathWinding(ctx, (int)ctx->commands[i+1]);
                i += 2;
                break;
            default:
                i++;
        }
    }
    
    cache->bounds[0] = cache->bounds[1] = 1e6f;
    cache->bounds[2] = cache->bounds[3] = -1e6f;
    
    // Calculate the direction and length of line segments.
    for (j = 0; j < cache->npaths; j++) {
        path = &cache->paths[j];
        pts = &cache->points[path->first];
        
        // If the first and last points are the same, remove the last, mark as closed path.
        p0 = &pts[path->count-1];
        p1 = &pts[0];
        if (nvg__ptEquals(p0->x,p0->y, p1->x,p1->y, ctx->distTol)) {
            path->count--;
            p0 = &pts[path->count-1];
            path->closed = 1;
        }
        
        // Enforce winding.
        if (path->count > 2) {
            area = nvg__polyArea(pts, path->count);
            if (path->winding == NVG_CCW && area < 0.0f)
                nvg__polyReverse(pts, path->count);
            if (path->winding == NVG_CW && area > 0.0f)
                nvg__polyReverse(pts, path->count);
        }
        
        for(i = 0; i < path->count; i++) {
            // Calculate segment direction and length
            p0->dx = p1->x - p0->x;
            p0->dy = p1->y - p0->y;
            p0->len = nvg__normalize(&p0->dx, &p0->dy);
            // Update bounds
            cache->bounds[0] = nvg__minf(cache->bounds[0], p0->x);
            cache->bounds[1] = nvg__minf(cache->bounds[1], p0->y);
            cache->bounds[2] = nvg__maxf(cache->bounds[2], p0->x);
            cache->bounds[3] = nvg__maxf(cache->bounds[3], p0->y);
            // Advance
            p0 = p1++;
        }
    }
}

static int nvg__curveDivs(float r, float arc, float tol)
{
    float da = acosf(r / (r + tol)) * 2.0f;
    return nvg__maxi(2, (int)ceilf(arc / da));
}

static void nvg__chooseBevel(int bevel, NanoVG::NVGpoint* p0, NanoVG::NVGpoint* p1, float w,
                             float* x0, float* y0, float* x1, float* y1)
{
    if (bevel) {
        *x0 = p1->x + p0->dy * w;
        *y0 = p1->y - p0->dx * w;
        *x1 = p1->x + p1->dy * w;
        *y1 = p1->y - p1->dx * w;
    } else {
        *x0 = p1->x + p1->dmx * w;
        *y0 = p1->y + p1->dmy * w;
        *x1 = p1->x + p1->dmx * w;
        *y1 = p1->y + p1->dmy * w;
    }
}

static NanoVG::NVGvertex* nvg__roundJoin(NanoVG::NVGvertex* dst, NanoVG::NVGpoint* p0, NanoVG::NVGpoint* p1,
                                         float lw, float rw, float lu, float ru, int ncap, float fringe)
{
    int i, n;
    float dlx0 = p0->dy;
    float dly0 = -p0->dx;
    float dlx1 = p1->dy;
    float dly1 = -p1->dx;
    NVG_NOTUSED(fringe);
    
    if (p1->flags & NVG_PT_LEFT) {
        float lx0,ly0,lx1,ly1,a0,a1;
        nvg__chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, lw, &lx0,&ly0, &lx1,&ly1);
        a0 = atan2f(-dly0, -dlx0);
        a1 = atan2f(-dly1, -dlx1);
        if (a1 > a0) a1 -= NVG_PI*2;
        
        nvg__vset(dst, lx0, ly0, lu,1); dst++;
        nvg__vset(dst, p1->x - dlx0*rw, p1->y - dly0*rw, ru,1); dst++;
        
        n = nvg__clampi((int)ceilf(((a0 - a1) / NVG_PI) * ncap), 2, ncap);
        for (i = 0; i < n; i++) {
            float u = i/(float)(n-1);
            float a = a0 + u*(a1-a0);
            float rx = p1->x + cosf(a) * rw;
            float ry = p1->y + sinf(a) * rw;
            nvg__vset(dst, p1->x, p1->y, 0.5f,1); dst++;
            nvg__vset(dst, rx, ry, ru,1); dst++;
        }
        
        nvg__vset(dst, lx1, ly1, lu,1); dst++;
        nvg__vset(dst, p1->x - dlx1*rw, p1->y - dly1*rw, ru,1); dst++;
        
    } else {
        float rx0,ry0,rx1,ry1,a0,a1;
        nvg__chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, -rw, &rx0,&ry0, &rx1,&ry1);
        a0 = atan2f(dly0, dlx0);
        a1 = atan2f(dly1, dlx1);
        if (a1 < a0) a1 += NVG_PI*2;
        
        nvg__vset(dst, p1->x + dlx0*rw, p1->y + dly0*rw, lu,1); dst++;
        nvg__vset(dst, rx0, ry0, ru,1); dst++;
        
        n = nvg__clampi((int)ceilf(((a1 - a0) / NVG_PI) * ncap), 2, ncap);
        for (i = 0; i < n; i++) {
            float u = i/(float)(n-1);
            float a = a0 + u*(a1-a0);
            float lx = p1->x + cosf(a) * lw;
            float ly = p1->y + sinf(a) * lw;
            nvg__vset(dst, lx, ly, lu,1); dst++;
            nvg__vset(dst, p1->x, p1->y, 0.5f,1); dst++;
        }
        
        nvg__vset(dst, p1->x + dlx1*rw, p1->y + dly1*rw, lu,1); dst++;
        nvg__vset(dst, rx1, ry1, ru,1); dst++;
        
    }
    return dst;
}

static NanoVG::NVGvertex* nvg__bevelJoin(NanoVG::NVGvertex* dst, NanoVG::NVGpoint* p0, NanoVG::NVGpoint* p1,
                                         float lw, float rw, float lu, float ru, float fringe)
{
    float rx0,ry0,rx1,ry1;
    float lx0,ly0,lx1,ly1;
    float dlx0 = p0->dy;
    float dly0 = -p0->dx;
    float dlx1 = p1->dy;
    float dly1 = -p1->dx;
    NVG_NOTUSED(fringe);
    
    if (p1->flags & NVG_PT_LEFT) {
        nvg__chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, lw, &lx0,&ly0, &lx1,&ly1);
        
        nvg__vset(dst, lx0, ly0, lu,1); dst++;
        nvg__vset(dst, p1->x - dlx0*rw, p1->y - dly0*rw, ru,1); dst++;
        
        if (p1->flags & NVG_PT_BEVEL) {
            nvg__vset(dst, lx0, ly0, lu,1); dst++;
            nvg__vset(dst, p1->x - dlx0*rw, p1->y - dly0*rw, ru,1); dst++;
            
            nvg__vset(dst, lx1, ly1, lu,1); dst++;
            nvg__vset(dst, p1->x - dlx1*rw, p1->y - dly1*rw, ru,1); dst++;
        } else {
            rx0 = p1->x - p1->dmx * rw;
            ry0 = p1->y - p1->dmy * rw;
            
            nvg__vset(dst, p1->x, p1->y, 0.5f,1); dst++;
            nvg__vset(dst, p1->x - dlx0*rw, p1->y - dly0*rw, ru,1); dst++;
            
            nvg__vset(dst, rx0, ry0, ru,1); dst++;
            nvg__vset(dst, rx0, ry0, ru,1); dst++;
            
            nvg__vset(dst, p1->x, p1->y, 0.5f,1); dst++;
            nvg__vset(dst, p1->x - dlx1*rw, p1->y - dly1*rw, ru,1); dst++;
        }
        
        nvg__vset(dst, lx1, ly1, lu,1); dst++;
        nvg__vset(dst, p1->x - dlx1*rw, p1->y - dly1*rw, ru,1); dst++;
        
    } else {
        nvg__chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, -rw, &rx0,&ry0, &rx1,&ry1);
        
        nvg__vset(dst, p1->x + dlx0*lw, p1->y + dly0*lw, lu,1); dst++;
        nvg__vset(dst, rx0, ry0, ru,1); dst++;
        
        if (p1->flags & NVG_PT_BEVEL) {
            nvg__vset(dst, p1->x + dlx0*lw, p1->y + dly0*lw, lu,1); dst++;
            nvg__vset(dst, rx0, ry0, ru,1); dst++;
            
            nvg__vset(dst, p1->x + dlx1*lw, p1->y + dly1*lw, lu,1); dst++;
            nvg__vset(dst, rx1, ry1, ru,1); dst++;
        } else {
            lx0 = p1->x + p1->dmx * lw;
            ly0 = p1->y + p1->dmy * lw;
            
            nvg__vset(dst, p1->x + dlx0*lw, p1->y + dly0*lw, lu,1); dst++;
            nvg__vset(dst, p1->x, p1->y, 0.5f,1); dst++;
            
            nvg__vset(dst, lx0, ly0, lu,1); dst++;
            nvg__vset(dst, lx0, ly0, lu,1); dst++;
            
            nvg__vset(dst, p1->x + dlx1*lw, p1->y + dly1*lw, lu,1); dst++;
            nvg__vset(dst, p1->x, p1->y, 0.5f,1); dst++;
        }
        
        nvg__vset(dst, p1->x + dlx1*lw, p1->y + dly1*lw, lu,1); dst++;
        nvg__vset(dst, rx1, ry1, ru,1); dst++;
    }
    
    return dst;
}

static NanoVG::NVGvertex* nvg__buttCapStart(NanoVG::NVGvertex* dst, NanoVG::NVGpoint* p,
                                            float dx, float dy, float w, float d, float aa)
{
    float px = p->x - dx*d;
    float py = p->y - dy*d;
    float dlx = dy;
    float dly = -dx;
    nvg__vset(dst, px + dlx*w - dx*aa, py + dly*w - dy*aa, 0,0); dst++;
    nvg__vset(dst, px - dlx*w - dx*aa, py - dly*w - dy*aa, 1,0); dst++;
    nvg__vset(dst, px + dlx*w, py + dly*w, 0,1); dst++;
    nvg__vset(dst, px - dlx*w, py - dly*w, 1,1); dst++;
    return dst;
}

static NanoVG::NVGvertex* nvg__buttCapEnd(NanoVG::NVGvertex* dst, NanoVG::NVGpoint* p,
                                          float dx, float dy, float w, float d, float aa)
{
    float px = p->x + dx*d;
    float py = p->y + dy*d;
    float dlx = dy;
    float dly = -dx;
    nvg__vset(dst, px + dlx*w, py + dly*w, 0,1); dst++;
    nvg__vset(dst, px - dlx*w, py - dly*w, 1,1); dst++;
    nvg__vset(dst, px + dlx*w + dx*aa, py + dly*w + dy*aa, 0,0); dst++;
    nvg__vset(dst, px - dlx*w + dx*aa, py - dly*w + dy*aa, 1,0); dst++;
    return dst;
}


static NanoVG::NVGvertex* nvg__roundCapStart(NanoVG::NVGvertex* dst, NanoVG::NVGpoint* p,
                                             float dx, float dy, float w, int ncap, float aa)
{
    int i;
    float px = p->x;
    float py = p->y;
    float dlx = dy;
    float dly = -dx;
    NVG_NOTUSED(aa);
    for (i = 0; i < ncap; i++) {
        float a = i/(float)(ncap-1)*NVG_PI;
        float ax = cosf(a) * w, ay = sinf(a) * w;
        nvg__vset(dst, px - dlx*ax - dx*ay, py - dly*ax - dy*ay, 0,1); dst++;
        nvg__vset(dst, px, py, 0.5f,1); dst++;
    }
    nvg__vset(dst, px + dlx*w, py + dly*w, 0,1); dst++;
    nvg__vset(dst, px - dlx*w, py - dly*w, 1,1); dst++;
    return dst;
}

static NanoVG::NVGvertex* nvg__roundCapEnd(NanoVG::NVGvertex* dst, NanoVG::NVGpoint* p,
                                           float dx, float dy, float w, int ncap, float aa)
{
    int i;
    float px = p->x;
    float py = p->y;
    float dlx = dy;
    float dly = -dx;
    NVG_NOTUSED(aa);
    nvg__vset(dst, px + dlx*w, py + dly*w, 0,1); dst++;
    nvg__vset(dst, px - dlx*w, py - dly*w, 1,1); dst++;
    for (i = 0; i < ncap; i++) {
        float a = i/(float)(ncap-1)*NVG_PI;
        float ax = cosf(a) * w, ay = sinf(a) * w;
        nvg__vset(dst, px, py, 0.5f,1); dst++;
        nvg__vset(dst, px - dlx*ax + dx*ay, py - dly*ax + dy*ay, 0,1); dst++;
    }
    return dst;
}


static void nvg__calculateJoins(NanoVG::NVGcontext* ctx, float w, int lineJoin, float miterLimit)
{
    NanoVG::NVGpathCache* cache = ctx->cache;
    int i, j;
    float iw = 0.0f;
    
    if (w > 0.0f) iw = 1.0f / w;
    
    // Calculate which joins needs extra vertices to append, and gather vertex count.
    for (i = 0; i < cache->npaths; i++) {
        NanoVG::NVGpath* path = &cache->paths[i];
        NanoVG::NVGpoint* pts = &cache->points[path->first];
        NanoVG::NVGpoint* p0 = &pts[path->count-1];
        NanoVG::NVGpoint* p1 = &pts[0];
        int nleft = 0;
        
        path->nbevel = 0;
        
        for (j = 0; j < path->count; j++) {
            float dlx0, dly0, dlx1, dly1, dmr2, cross, limit;
            
            dlx0 = p0->dy;
            dly0 = -p0->dx;
            
            dlx1 = p1->dy;
            dly1 = -p1->dx;
            
            // Calculate extrusions
            p1->dmx = (dlx0 + dlx1) * 0.5f;
            p1->dmy = (dly0 + dly1) * 0.5f;
            
            dmr2 = p1->dmx*p1->dmx + p1->dmy*p1->dmy;
            if (dmr2 > 0.000001f) {
                float scale = 1.0f / dmr2;
                if (scale > 600.0f) {
                    scale = 600.0f;
                }
                p1->dmx *= scale;
                p1->dmy *= scale;
            }
            
            // Clear flags, but keep the corner.
            p1->flags = (p1->flags & NVG_PT_CORNER) ? NVG_PT_CORNER : 0;
            
            // Keep track of left turns.
            cross = p1->dx * p0->dy - p0->dx * p1->dy;
            if (cross > 0.0f) {
                nleft++;
                p1->flags |= NVG_PT_LEFT;
            }
            
            // Calculate if we should use bevel or miter for inner join.
            limit = nvg__maxf(1.01f, nvg__minf(p0->len, p1->len) * iw);
            if ((dmr2 * limit*limit) < 1.0f)
                p1->flags |= NVG_PR_INNERBEVEL;
            
            // Check to see if the corner needs to be beveled.
            if (p1->flags & NVG_PT_CORNER) {
                if ((dmr2 * miterLimit*miterLimit) < 1.0f || lineJoin == NanoVG::NVG_BEVEL || lineJoin == NanoVG::NVG_ROUND) {
                    p1->flags |= NVG_PT_BEVEL;
                }
            }
            
            if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
                path->nbevel++;
            
            p0 = p1++;
        }
        
        path->convex = (nleft == path->count) ? 1 : 0;
    }
}


static int nvg__expandStroke(NanoVG::NVGcontext* ctx, float w, int lineCap, int lineJoin, float miterLimit)
{
    NanoVG::NVGpathCache* cache = ctx->cache;
    NanoVG::NVGvertex* verts;
    NanoVG::NVGvertex* dst;
    int cverts, i, j;
    float aa = ctx->fringeWidth;
    int ncap = nvg__curveDivs(w, NVG_PI, ctx->tessTol);    // Calculate divisions per half circle.
    
    nvg__calculateJoins(ctx, w, lineJoin, miterLimit);
    
    // Calculate max vertex usage.
    cverts = 0;
    for (i = 0; i < cache->npaths; i++) {
        NanoVG::NVGpath* path = &cache->paths[i];
        int loop = (path->closed == 0) ? 0 : 1;
        if (lineCap == NanoVG::NVG_ROUND)
            cverts += (path->count + path->nbevel*(ncap+2) + 1) * 2; // plus one for loop
        else
            cverts += (path->count + path->nbevel*5 + 1) * 2; // plus one for loop
        if (loop == 0) {
            // space for caps
            if (lineCap == NanoVG::NVG_ROUND) {
                cverts += (ncap*2 + 2)*2;
            } else {
                cverts += (3+3)*2;
            }
        }
    }
    
    verts = nvg__allocTempVerts(ctx, cverts);
    if (verts == NULL) return 0;
    
    for (i = 0; i < cache->npaths; i++) {
        NanoVG::NVGpath* path = &cache->paths[i];
        NanoVG::NVGpoint* pts = &cache->points[path->first];
        NanoVG::NVGpoint* p0;
        NanoVG::NVGpoint* p1;
        int s, e, loop;
        float dx, dy;
        
        path->fill = 0;
        path->nfill = 0;
        
        // Calculate fringe or stroke
        loop = (path->closed == 0) ? 0 : 1;
        dst = verts;
        path->stroke = dst;
        
        if (loop) {
            // Looping
            p0 = &pts[path->count-1];
            p1 = &pts[0];
            s = 0;
            e = path->count;
        } else {
            // Add cap
            p0 = &pts[0];
            p1 = &pts[1];
            s = 1;
            e = path->count-1;
        }
        
        if (loop == 0) {
            // Add cap
            dx = p1->x - p0->x;
            dy = p1->y - p0->y;
            nvg__normalize(&dx, &dy);
            if (lineCap == NanoVG::NVG_BUTT)
                dst = nvg__buttCapStart(dst, p0, dx, dy, w, -aa*0.5f, aa);
            else if (lineCap == NanoVG::NVG_BUTT || lineCap == NanoVG::NVG_SQUARE)
                dst = nvg__buttCapStart(dst, p0, dx, dy, w, w-aa, aa);
            else if (lineCap == NanoVG::NVG_ROUND)
                dst = nvg__roundCapStart(dst, p0, dx, dy, w, ncap, aa);
        }
        
        for (j = s; j < e; ++j) {
            if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0) {
                if (lineJoin == NanoVG::NVG_ROUND) {
                    dst = nvg__roundJoin(dst, p0, p1, w, w, 0, 1, ncap, aa);
                } else {
                    dst = nvg__bevelJoin(dst, p0, p1, w, w, 0, 1, aa);
                }
            } else {
                nvg__vset(dst, p1->x + (p1->dmx * w), p1->y + (p1->dmy * w), 0,1); dst++;
                nvg__vset(dst, p1->x - (p1->dmx * w), p1->y - (p1->dmy * w), 1,1); dst++;
            }
            p0 = p1++;
        }
        
        if (loop) {
            // Loop it
            nvg__vset(dst, verts[0].x, verts[0].y, 0,1); dst++;
            nvg__vset(dst, verts[1].x, verts[1].y, 1,1); dst++;
        } else {
            // Add cap
            dx = p1->x - p0->x;
            dy = p1->y - p0->y;
            nvg__normalize(&dx, &dy);
            if (lineCap == NanoVG::NVG_BUTT)
                dst = nvg__buttCapEnd(dst, p1, dx, dy, w, -aa*0.5f, aa);
            else if (lineCap == NanoVG::NVG_BUTT || lineCap == NanoVG::NVG_SQUARE)
                dst = nvg__buttCapEnd(dst, p1, dx, dy, w, w-aa, aa);
            else if (lineCap == NanoVG::NVG_ROUND)
                dst = nvg__roundCapEnd(dst, p1, dx, dy, w, ncap, aa);
        }
        
        path->nstroke = (int)(dst - verts);
        
        verts = dst;
    }
    
    return 1;
}

static int nvg__expandFill(NanoVG::NVGcontext* ctx, float w, int lineJoin, float miterLimit)
{
    NanoVG::NVGpathCache* cache = ctx->cache;
    NanoVG::NVGvertex* verts;
    NanoVG::NVGvertex* dst;
    int cverts, convex, i, j;
    float aa = ctx->fringeWidth;
    int fringe = w > 0.0f;
    
    nvg__calculateJoins(ctx, w, lineJoin, miterLimit);
    
    // Calculate max vertex usage.
    cverts = 0;
    for (i = 0; i < cache->npaths; i++) {
        NanoVG::NVGpath* path = &cache->paths[i];
        cverts += path->count + path->nbevel + 1;
        if (fringe)
            cverts += (path->count + path->nbevel*5 + 1) * 2; // plus one for loop
    }
    
    verts = nvg__allocTempVerts(ctx, cverts);
    if (verts == NULL) return 0;
    
    convex = cache->npaths == 1 && cache->paths[0].convex;
    
    for (i = 0; i < cache->npaths; i++) {
        NanoVG::NVGpath* path = &cache->paths[i];
        NanoVG::NVGpoint* pts = &cache->points[path->first];
        NanoVG::NVGpoint* p0;
        NanoVG::NVGpoint* p1;
        float rw, lw, woff;
        float ru, lu;
        
        // Calculate shape vertices.
        woff = 0.5f*aa;
        dst = verts;
        path->fill = dst;
        
        if (fringe) {
            // Looping
            p0 = &pts[path->count-1];
            p1 = &pts[0];
            for (j = 0; j < path->count; ++j) {
                if (p1->flags & NVG_PT_BEVEL) {
                    float dlx0 = p0->dy;
                    float dly0 = -p0->dx;
                    float dlx1 = p1->dy;
                    float dly1 = -p1->dx;
                    if (p1->flags & NVG_PT_LEFT) {
                        float lx = p1->x + p1->dmx * woff;
                        float ly = p1->y + p1->dmy * woff;
                        nvg__vset(dst, lx, ly, 0.5f,1); dst++;
                    } else {
                        float lx0 = p1->x + dlx0 * woff;
                        float ly0 = p1->y + dly0 * woff;
                        float lx1 = p1->x + dlx1 * woff;
                        float ly1 = p1->y + dly1 * woff;
                        nvg__vset(dst, lx0, ly0, 0.5f,1); dst++;
                        nvg__vset(dst, lx1, ly1, 0.5f,1); dst++;
                    }
                } else {
                    nvg__vset(dst, p1->x + (p1->dmx * woff), p1->y + (p1->dmy * woff), 0.5f,1); dst++;
                }
                p0 = p1++;
            }
        } else {
            for (j = 0; j < path->count; ++j) {
                nvg__vset(dst, pts[j].x, pts[j].y, 0.5f,1);
                dst++;
            }
        }
        
        path->nfill = (int)(dst - verts);
        verts = dst;
        
        // Calculate fringe
        if (fringe) {
            lw = w + woff;
            rw = w - woff;
            lu = 0;
            ru = 1;
            dst = verts;
            path->stroke = dst;
            
            // Create only half a fringe for convex shapes so that
            // the shape can be rendered without stenciling.
            if (convex) {
                lw = woff;    // This should generate the same vertex as fill inset above.
                lu = 0.5f;    // Set outline fade at middle.
            }
            
            // Looping
            p0 = &pts[path->count-1];
            p1 = &pts[0];
            
            for (j = 0; j < path->count; ++j) {
                if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0) {
                    dst = nvg__bevelJoin(dst, p0, p1, lw, rw, lu, ru, ctx->fringeWidth);
                } else {
                    nvg__vset(dst, p1->x + (p1->dmx * lw), p1->y + (p1->dmy * lw), lu,1); dst++;
                    nvg__vset(dst, p1->x - (p1->dmx * rw), p1->y - (p1->dmy * rw), ru,1); dst++;
                }
                p0 = p1++;
            }
            
            // Loop it
            nvg__vset(dst, verts[0].x, verts[0].y, lu,1); dst++;
            nvg__vset(dst, verts[1].x, verts[1].y, ru,1); dst++;
            
            path->nstroke = (int)(dst - verts);
            verts = dst;
        } else {
            path->stroke = NULL;
            path->nstroke = 0;
        }
    }
    
    return 1;
}


//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// Draw
//------------------------------------------------------------------------------------------------
void NanoVG::nvgBeginPath()
{
    _context->ncommands = 0;
    nvg__clearPathCache(_context);
}

void NanoVG::nvgMoveTo(float x, float y)
{
    float vals[] = { NVG_MOVETO, x, y };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgLineTo(float x, float y)
{
    float vals[] = { NVG_LINETO, x, y };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgBezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y)
{
    float vals[] = { NVG_BEZIERTO, c1x, c1y, c2x, c2y, x, y };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgQuadTo(float cx, float cy, float x, float y)
{
    float x0 = _context->commandx;
    float y0 = _context->commandy;
    float vals[] = { NVG_BEZIERTO,
        x0 + 2.0f/3.0f*(cx - x0), y0 + 2.0f/3.0f*(cy - y0),
        x + 2.0f/3.0f*(cx - x), y + 2.0f/3.0f*(cy - y),
        x, y };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgArcTo(float x1, float y1, float x2, float y2, float radius)
{
    float x0 = _context->commandx;
    float y0 = _context->commandy;
    float dx0,dy0, dx1,dy1, a, d, cx,cy, a0,a1;
    int dir;
    
    if (_context->ncommands == 0) {
        return;
    }
    
    // Handle degenerate cases.
    if (nvg__ptEquals(x0,y0, x1,y1, _context->distTol) ||
        nvg__ptEquals(x1,y1, x2,y2, _context->distTol) ||
        nvg__distPtSeg(x1,y1, x0,y0, x2,y2) < _context->distTol*_context->distTol ||
        radius < _context->distTol) {
        nvgLineTo(x1,y1);
        return;
    }
    
    // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
    dx0 = x0-x1;
    dy0 = y0-y1;
    dx1 = x2-x1;
    dy1 = y2-y1;
    nvg__normalize(&dx0,&dy0);
    nvg__normalize(&dx1,&dy1);
    a = nvg__acosf(dx0*dx1 + dy0*dy1);
    d = radius / nvg__tanf(a/2.0f);
    
    //    printf("a=%f° d=%f\n", a/NVG_PI*180.0f, d);
    
    if (d > 10000.0f) {
        nvgLineTo(x1,y1);
        return;
    }
    
    if (nvg__cross(dx0,dy0, dx1,dy1) > 0.0f) {
        cx = x1 + dx0*d + dy0*radius;
        cy = y1 + dy0*d + -dx0*radius;
        a0 = nvg__atan2f(dx0, -dy0);
        a1 = nvg__atan2f(-dx1, dy1);
        dir = NVG_CW;
        //        printf("CW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy, a0/NVG_PI*180.0f, a1/NVG_PI*180.0f);
    } else {
        cx = x1 + dx0*d + -dy0*radius;
        cy = y1 + dy0*d + dx0*radius;
        a0 = nvg__atan2f(-dx0, dy0);
        a1 = nvg__atan2f(dx1, -dy1);
        dir = NVG_CCW;
        //        printf("CCW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy, a0/NVG_PI*180.0f, a1/NVG_PI*180.0f);
    }
    
    nvgArc(cx, cy, radius, a0, a1, dir);
}

void NanoVG::nvgClosePath()
{
    float vals[] = { NVG_CLOSE };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgPathWinding(int dir)
{
    float vals[] = { NVG_WINDING, (float)dir };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgArc(float cx, float cy, float r, float a0, float a1, int dir)
{
    float a = 0, da = 0, hda = 0, kappa = 0;
    float dx = 0, dy = 0, x = 0, y = 0, tanx = 0, tany = 0;
    float px = 0, py = 0, ptanx = 0, ptany = 0;
    float vals[3 + 5*7 + 100];
    int i, ndivs, nvals;
    int move = _context->ncommands > 0 ? NVG_LINETO : NVG_MOVETO;
    
    // Clamp angles
    da = a1 - a0;
    if (dir == NVG_CW) {
        if (nvg__absf(da) >= NVG_PI*2) {
            da = NVG_PI*2;
        } else {
            while (da < 0.0f) da += NVG_PI*2;
        }
    } else {
        if (nvg__absf(da) >= NVG_PI*2) {
            da = -NVG_PI*2;
        } else {
            while (da > 0.0f) da -= NVG_PI*2;
        }
    }
    
    // Split arc into max 90 degree segments.
    ndivs = nvg__maxi(1, nvg__mini((int)(nvg__absf(da) / (NVG_PI*0.5f) + 0.5f), 5));
    hda = (da / (float)ndivs) / 2.0f;
    kappa = nvg__absf(4.0f / 3.0f * (1.0f - nvg__cosf(hda)) / nvg__sinf(hda));
    
    if (dir == NVG_CCW)
        kappa = -kappa;
    
    nvals = 0;
    for (i = 0; i <= ndivs; i++) {
        a = a0 + da * (i/(float)ndivs);
        dx = nvg__cosf(a);
        dy = nvg__sinf(a);
        x = cx + dx*r;
        y = cy + dy*r;
        tanx = -dy*r*kappa;
        tany = dx*r*kappa;
        
        if (i == 0) {
            vals[nvals++] = (float)move;
            vals[nvals++] = x;
            vals[nvals++] = y;
        } else {
            vals[nvals++] = NVG_BEZIERTO;
            vals[nvals++] = px+ptanx;
            vals[nvals++] = py+ptany;
            vals[nvals++] = x-tanx;
            vals[nvals++] = y-tany;
            vals[nvals++] = x;
            vals[nvals++] = y;
        }
        px = x;
        py = y;
        ptanx = tanx;
        ptany = tany;
    }
    
    nvg__appendCommands(_context, vals, nvals);
}

void NanoVG::nvgRect(float x, float y, float w, float h)
{
    float vals[] = {
        NVG_MOVETO, x,y,
        NVG_LINETO, x,y+h,
        NVG_LINETO, x+w,y+h,
        NVG_LINETO, x+w,y,
        NVG_CLOSE
    };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgRoundedRect(float x, float y, float w, float h, float r)
{
    if (r < 0.1f) {
        nvgRect(x,y,w,h);
        return;
    }
    else {
        float rx = nvg__minf(r, nvg__absf(w)*0.5f) * nvg__signf(w), ry = nvg__minf(r, nvg__absf(h)*0.5f) * nvg__signf(h);
        float vals[] = {
            NVG_MOVETO, x, y+ry,
            NVG_LINETO, x, y+h-ry,
            NVG_BEZIERTO, x, y+h-ry*(1-NVG_KAPPA90), x+rx*(1-NVG_KAPPA90), y+h, x+rx, y+h,
            NVG_LINETO, x+w-rx, y+h,
            NVG_BEZIERTO, x+w-rx*(1-NVG_KAPPA90), y+h, x+w, y+h-ry*(1-NVG_KAPPA90), x+w, y+h-ry,
            NVG_LINETO, x+w, y+ry,
            NVG_BEZIERTO, x+w, y+ry*(1-NVG_KAPPA90), x+w-rx*(1-NVG_KAPPA90), y, x+w-rx, y,
            NVG_LINETO, x+rx, y,
            NVG_BEZIERTO, x+rx*(1-NVG_KAPPA90), y, x, y+ry*(1-NVG_KAPPA90), x, y+ry,
            NVG_CLOSE
        };
        nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
    }
}

void NanoVG::nvgEllipse(float cx, float cy, float rx, float ry)
{
    float vals[] = {
        NVG_MOVETO, cx-rx, cy,
        NVG_BEZIERTO, cx-rx, cy+ry*NVG_KAPPA90, cx-rx*NVG_KAPPA90, cy+ry, cx, cy+ry,
        NVG_BEZIERTO, cx+rx*NVG_KAPPA90, cy+ry, cx+rx, cy+ry*NVG_KAPPA90, cx+rx, cy,
        NVG_BEZIERTO, cx+rx, cy-ry*NVG_KAPPA90, cx+rx*NVG_KAPPA90, cy-ry, cx, cy-ry,
        NVG_BEZIERTO, cx-rx*NVG_KAPPA90, cy-ry, cx-rx, cy-ry*NVG_KAPPA90, cx-rx, cy,
        NVG_CLOSE
    };
    nvg__appendCommands(_context, vals, NVG_COUNTOF(vals));
}

void NanoVG::nvgCircle(float cx, float cy, float r)
{
    nvgEllipse(cx,cy, r,r);
}

void NanoVG::nvgFill()
{
    nvg__flattenPaths(_context);
    if (_context->params.edgeAntiAlias) {
        nvg__expandFill(_context, _context->fringeWidth, NVG_MITER, 2.4f);
    } else {
        nvg__expandFill(_context, 0.0f, NVG_MITER, 2.4f);
    }
}

void NanoVG::nvgStroke()
{
    NVGstate* state = nvg__getState(_context);
    float scale = nvg__getAverageScale(state->xform);
    float strokeWidth = nvg__clampf(state->strokeWidth * scale, 0.0f, 200.0f);
    
    if (strokeWidth < _context->fringeWidth) {
        // If the stroke width is less than pixel size, use alpha to emulate coverage.
        // Since coverage is area, scale by alpha*alpha.
        strokeWidth = _context->fringeWidth;
    }
    
    nvg__flattenPaths(_context);
    
    if (_context->params.edgeAntiAlias) {
        nvg__expandStroke(_context, strokeWidth*0.5f + _context->fringeWidth*0.5f, state->lineCap, state->lineJoin, state->miterLimit);
    } else {
        nvg__expandStroke(_context, strokeWidth*0.5f, state->lineCap, state->lineJoin, state->miterLimit);
    }
}



