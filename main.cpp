#include <iostream>
#include "SDL.h"
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800
#define MARKER_SIZE 10.0f
#define FPS 60
#define DELAY_SEC (1.0f/FPS) // delay in seconds
#define DELAY_MS (Uint32) floorf(DELAY_SEC*1000) // delay in milliseconds
#define RED Color(255,0,0,255)
#define GREEN Color(0,255,0,255)
#define BLUE Color(33,150,243,180)
#define DARKBLUE Color(0,105,192,255)

#define BLACK Color(0,0,0,255)
#define  WHITE Color(255,255,255,255)
#define CAPACITY 256


using namespace  std;

//error handling
int check_sd_code(int code){
    if(code< 0){
        cerr<<stderr<<"SDL error : "<<SDL_GetError();
        exit(1);
    }
    return code;
}
// error handling
SDL_Window* check_sdl_pointer(SDL_Window* ptr){
    if(ptr== nullptr) {
        cerr << stderr << "SDL pointer error :" << SDL_GetError();
        exit(0);
    }
    return ptr;
}

// a utility vector that makes passing vectors easy
struct Vec2{
    float x;
    float y;
    Vec2(float x,float y){
        this->x = x;
        this->y = y;
    }
    Vec2(){
        this->x = 0;
        this->y = 0;
    }
    Vec2 operator + (Vec2 const &obj) const{
        return Vec2(x+obj.x,y+obj.y);
    }
    Vec2 operator - (Vec2 const &obj) const{
        return Vec2(x-obj.x,y-obj.y);
    }
    Vec2 operator * (float const &obj) const{
        return Vec2(x*obj,y*obj);
    }
};

// a utility structure to make passing colors easy
struct Color{

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    Color(Uint8 r , Uint8 g,Uint8 b,Uint8 a){
        this->r = r; this ->g = g; this->b = b; this->a = a;
    }


};

// a function that renders a line in the screen
void render_line(SDL_Renderer *renderer , Vec2 begin,Vec2 end,Color color){

    SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,color.a);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_MUL);
    SDL_RenderDrawLine(renderer,(int)begin.x,(int)begin.y,(int)end.x,(int)end.y);

}

// a function to create a rectangle at a given position and a given size
void fillRect(SDL_Renderer *renderer , Vec2 pos,Vec2 size,Color color = Color(255,0,0,0)){
    SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,color.a);
    const SDL_Rect rect = {(int)pos.x,(int)pos.y,(int)size.x,(int)size.y};
    SDL_RenderFillRect(renderer,&rect);

}

// linear interpolation program
Vec2 lerp(Vec2 a , Vec2 b,float p){
    return a*(1-p)+b*p;
}

// marker
void render_marker(SDL_Renderer * renderer , Vec2 pos ,Color color){
    Vec2 temp = Vec2(MARKER_SIZE,MARKER_SIZE);
    fillRect(renderer,pos - temp*0.5,temp,color);
}

// gives us the resultant vector in curve after iteratively applying lerps
// to form a n degree bezeir curve
Vec2 bezSample(Vec2 *ps,size_t n , float p){
    Vec2 xs[n];
    memcpy(xs,ps,sizeof(Vec2)*n);
    while(n>1){
        for(size_t i =0;i<n-1;++i){
            xs[i] = lerp(xs[i],xs[i+1],p);
        }
        n--;
    }

    return xs[0];
}
/*// A cubic bezeir curve
 * void renderBezierMarkers(SDL_Renderer *renderer, Vec2 a, Vec2 b, Vec2 c,Vec2 d,float s,Color color = GREEN){
    Vec2 prevPosition = a;
    for(float  p =0;p<=1;p+=s){
        Vec2 ab = lerp(a,b,p);
        Vec2 bc = lerp(b,c,p);
        Vec2 cd = lerp(c,d,p);
        Vec2 abc = lerp(ab,bc,p);
        Vec2 bcd = lerp(bc,cd,p);
        Vec2 abcd = lerp(abc,bcd,p);
        //render_marker(renderer,abcd,color);
        render_line(renderer,prevPosition,abcd,BLACK);
        prevPosition = abcd;
    }
    render_line(renderer,prevPosition,d,BLACK);
}
*/
void renderBezierMarkers(SDL_Renderer *renderer, Vec2 *points,size_t numberOfPoints,float s,Color color = GREEN){
    Vec2 prevPosition = points[0];
    Vec2 finalPosition = points[numberOfPoints-1];

    for(float  p =0;p<=1;p+=s){
        Vec2 v = bezSample(points,numberOfPoints,p);
        //render_marker(renderer,abcd,color);
        // draw a connecting line to trace out the bezier curve
        render_line(renderer,prevPosition,v,BLACK);
        prevPosition = v;
    }
   // connecting the last two nodes in the curve
    render_line(renderer,prevPosition,finalPosition,BLACK);
}
Vec2 ps[CAPACITY];
size_t psCount = 0;
int pointSelected = -1;

// a function that would return which node was clicked by the user mouse
// returns -1 if the clicked area has no node
int ps_at(Vec2 pos){
    const Vec2 ps_size = Vec2(MARKER_SIZE,MARKER_SIZE);
    for(size_t i =0;i<psCount;++i){
        const Vec2 ps_begin = ps[i]-ps_size*0.5;
        const Vec2 ps_end = ps_size+ps_begin;
        if(ps_begin.x<=pos.x and pos.x<=ps_end.x and ps_begin.y<=pos.y and pos.y<=ps_end.y)
            return (int) i;
    }
    return -1;
}

int main() {

    check_sd_code(SDL_Init(SDL_INIT_VIDEO));
    // just create an empty window
    SDL_Window* const window = check_sdl_pointer(SDL_CreateWindow("Bezier Curve",0,0,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_RESIZABLE));
    // now we need a renderer for rendering
    SDL_Renderer * const renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer,SCREEN_WIDTH,SCREEN_HEIGHT);

    const Vec2 begin = Vec2(0,0);
    const Vec2 end = Vec2(SCREEN_WIDTH,SCREEN_HEIGHT);
    float p = 0;

    int quit = 0;


    // infinte loop till you quit
    float t = 0; // a global timer
    while(!quit){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                {
                    // putting in points and selcting them
                    switch (event.button.button){
                        case SDL_BUTTON_LEFT:{
                            Vec2 mousePosition = Vec2(event.button.x,event.button.y);
                            pointSelected = ps_at(mousePosition);
                            if(pointSelected<0)
                            ps[psCount++] = mousePosition;

                        }
                            break;
                    }

                }
                break;
                case SDL_MOUSEMOTION:
                {
                    // changing the position of points with mouse movement
                    Vec2 mousePosition = Vec2(event.motion.x,event.motion.y);
                    if(pointSelected>=0){
                        ps[pointSelected] = mousePosition;
                    }

                }
                    break;
                case SDL_MOUSEBUTTONUP:
                {
                    // deselcting a selected point
                    if(event.button.button == SDL_BUTTON_LEFT)
                        pointSelected = -1;
                }
                break;
            }
        }
        SDL_SetRenderDrawColor(renderer,255,255,255,255); // background color
        SDL_RenderClear(renderer);

        // for loop that marks all the markers created
        for(int i =0;psCount>0 and i<psCount;i++)
        render_marker(renderer,ps[i],DARKBLUE);

        // code that draws the handle lines for controlling the curve
        if(psCount>=4){
            for(int i =0;i<psCount-1;i++)
            render_line(renderer,ps[i],ps[i+1],BLUE);

        }


        // the code that traces out the bezeir curve
        if(psCount>=4)
        {
            renderBezierMarkers(renderer,ps,psCount,0.01,GREEN);
        }

       // presents your screen
       SDL_RenderPresent(renderer);


        SDL_Delay(DELAY_MS);
        t += DELAY_SEC;
        // adding time in sec to t after each frame

    }
    SDL_Quit();
    return 0;
}
