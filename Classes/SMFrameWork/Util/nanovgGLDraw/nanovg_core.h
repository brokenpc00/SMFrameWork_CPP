//
//  nanovg_core.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 27..
//

#ifndef nanovg_core_h
#define nanovg_core_h

class NanoVG {
public:
    struct NVGvertex {
        float x, y, u, v;
    };
    
    struct NVGpath {
        int first;
        int count;
        unsigned char closed;
        int nbevel;
        NVGvertex* fill;
        int nfill;
        NVGvertex* stroke;
        int nstroke;
        int winding;
        int convex;
    };
    
    enum NVGlineCap {
        NVG_BUTT,
        NVG_ROUND,
        NVG_SQUARE,
        NVG_BEVEL,
        NVG_MITER,
    };
    
    struct NVGparams {
        int edgeAntiAlias;
    };
    
    struct NVGstate {
        float strokeWidth;
        float miterLimit;
        int lineJoin;
        int lineCap;
        float xform[6];
    };
    
    struct NVGpoint {
        float x, y;
        float dx, dy;
        float len;
        float dmx, dmy;
        unsigned char flags;
    };
    
    struct NVGpathCache {
        NVGpoint* points;
        int npoints;
        int cpoints;
        NVGpath* paths;
        int npaths;
        int cpaths;
        NVGvertex* verts;
        int nverts;
        int cverts;
        float bounds[4];
    };
    
    
#define NVG_MAX_STATES 32
    struct NVGcontext {
        NVGparams params;
        float* commands;
        int ccommands;
        int ncommands;
        float commandx, commandy;
        NVGstate states[NVG_MAX_STATES];
        int nstates;
        NVGpathCache* cache;
        float tessTol;
        float distTol;
        float fringeWidth;
        float devicePxRatio;
    };

    static NanoVG* create();
    
    static void destroy(NanoVG* nano);
    
    NVGcontext* getContext() { return _context; }
    
    void nvgBeginFrame(float devicePixelRatio);
    
    void nvgSave();
    
    void nvgRestore();
    
    void nvgReset();
    
protected:
    NanoVG();
    
    ~NanoVG();
    
    bool init();
    
    void release();
    
public:
    void nvgMiterLimit(float limit);
    
    void nvgStrokeWidth(float size);
    
    void nvgLineCap(int cap);
    
    void nvgLineJoin(int join);
    
    void nvgBeginPath();
    
    void nvgMoveTo(float x, float y);
    
    void nvgLineTo(float x, float y);
    
    void nvgBezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y);
    
    void nvgQuadTo(float cx, float cy, float x, float y);
    
    void nvgArcTo(float x1, float y1, float x2, float y2, float radius);
    
    void nvgClosePath();
    
    void nvgPathWinding(int dir);
    
    void nvgArc(float cx, float cy, float r, float a0, float a1, int dir);
    
    void nvgRect(float x, float y, float w, float h);
    
    void nvgRoundedRect(float x, float y, float w, float h, float r);
    
    void nvgEllipse(float cx, float cy, float rx, float ry);
    
    void nvgCircle(float cx, float cy, float r);
    
    void nvgFill();
    
    void nvgStroke();
    
public:
    static void nvgTransformIdentity(float* dst);
    
    static void nvgTransformPoint(float* dstx, float* dsty, const float* xform, float srcx, float srcy);
    
private:
    NVGcontext* _context;
};


#endif /* nanovg_core_h */
