#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/RubiksCube.h"

RubiksCube cube;
Camera cam;
World3D world;
Number axis_line = 0;
int Mode = 0;
int Menu = 0;

void Menu_Set(int m){
	if(Menu==0 && m==1){
		AlxWindow_Mouse_SetInvisible(&window);
		SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
	}
	if(Menu==1 && m==0){
		AlxWindow_Mouse_SetVisible(&window);
	}
	
	Menu = m;
}
void Setup(AlxWindow* w){
	Menu_Set(1);

	cam = Camera_Make(
		(Vec3D){ 0.0f,0.0f,-5.0f,1.0f },
		(Vec3D){ 0.0f,0.0f,0.0f,1.0f },
		90.0f
	);

	world = World3D_Make(
		Matrix_MakeWorld((Vec3D){ 0.0f,0.0f,0.0f,1.0f },(Vec3D){ 0.0f,0.0f,0.0f,1.0f }),
		Matrix_MakePerspektive(cam.p,cam.up,cam.a),
		Matrix_MakeProjection(cam.fov,(float)GetHeight() / (float)GetWidth(),0.1f,1000.0f)
	);
	world.normal = WORLD3D_NORMAL_CAP;

	cube = RubiksCube_New(
		Vec3D_New(0.0f,0.0f,0.0f),
		(Pixel[]){ RED,GREEN,BLUE,YELLOW,ORANGE,WHITE }
	);
}
void Update(AlxWindow* w){
	if(Menu==1){
		Camera_Focus_S(&cam,GetMouseBefore(),GetMouse(),GetScreenRect().d,1.0f);
		cube.ax = -cam.a.x;
		cube.ay = -cam.a.y;
		
		//Camera_Update(&cam);
		SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
	}
	
	if(Stroke(ALX_KEY_W).DOWN)
		cam.p = Vec3D_Add(cam.p,Vec3D_Mul(cam.ld,1.0f * w->ElapsedTime));
	if(Stroke(ALX_KEY_S).DOWN)
		cam.p = Vec3D_Sub(cam.p,Vec3D_Mul(cam.ld,1.0f * w->ElapsedTime));
	if(Stroke(ALX_KEY_A).DOWN)
		cam.p = Vec3D_Add(cam.p,Vec3D_Mul(cam.sd,1.0f * w->ElapsedTime));
	if(Stroke(ALX_KEY_D).DOWN)
		cam.p = Vec3D_Sub(cam.p,Vec3D_Mul(cam.sd,1.0f * w->ElapsedTime));
	if(Stroke(ALX_KEY_R).DOWN)
		cam.p.y += 1.0f * w->ElapsedTime;
	if(Stroke(ALX_KEY_F).DOWN)
		cam.p.y -= 1.0f * w->ElapsedTime;
	
	if(Stroke(ALX_KEY_ESC).PRESSED)
		Menu_Set(!Menu);
	if(Stroke(ALX_KEY_Y).PRESSED)
		Mode = Mode < 3 ? Mode+1 : 0;

	if(Stroke(ALX_KEY_X).PRESSED)
		RubiksCube_Shuffle(&cube,1U);
	if(Stroke(ALX_KEY_C).PRESSED)
		RubiksCube_Unshuffle(&cube,1U);

	if(Stroke(ALX_KEY_SPACE).PRESSED)
		RubiksCube_Reset((RubiksCube_State*)&cube);

	if(Stroke(ALX_KEY_V).PRESSED)
		RubiksCube_Solve(&cube);

	if(Stroke(ALX_KEY_UP).PRESSED) 		axis_line = (axis_line + 1) % RUBIKSCUBE_SIDE_FIELDCOUNT;
	if(Stroke(ALX_KEY_DOWN).PRESSED) 	axis_line = (axis_line - 1 + RUBIKSCUBE_SIDE_FIELDCOUNT) % RUBIKSCUBE_SIDE_FIELDCOUNT;

	if(Stroke(ALX_KEY_T).PRESSED) 		RubiksCube_Rotate(&cube,0U,axis_line,0U);
	if(Stroke(ALX_KEY_G).PRESSED) 		RubiksCube_Rotate(&cube,0U,axis_line,1U);

	if(Stroke(ALX_KEY_Z).PRESSED) 		RubiksCube_Rotate(&cube,1U,axis_line,0U);
	if(Stroke(ALX_KEY_H).PRESSED) 		RubiksCube_Rotate(&cube,1U,axis_line,1U);

	if(Stroke(ALX_KEY_U).PRESSED) 		RubiksCube_Rotate(&cube,2U,axis_line,0U);
	if(Stroke(ALX_KEY_J).PRESSED) 		RubiksCube_Rotate(&cube,2U,axis_line,1U);


	World3D_Set_Model(&world,Matrix_MakeWorld((Vec3D){ 0.0f,0.0f,0.0f,1.0f },(Vec3D){ 0.0f,0.0f,0.0f,1.0f }));
	World3D_Set_View(&world,Matrix_MakePerspektive(cam.p,cam.up,Vec3D_New(0.0f,0.0f,0.0f)));
	World3D_Set_Proj(&world,Matrix_MakeProjection(cam.fov,(float)GetHeight() / (float)GetWidth(),0.1f,1000.0f));
	
	Vector_Clear(&world.trisIn);
	RubiksCube_Render(&cube,&world.trisIn);

	Clear(LIGHT_BLUE);

	World3D_Update(&world,cam.p,(Vec2){ GetWidth(),GetHeight() });

	for(int i = 0;i<world.trisOut.size;i++){
		Tri3D* t = (Tri3D*)Vector_Get(&world.trisOut,i);
		const Pixel c = Pixel_Mulf(t->c.c,t->c.l);

		if(Mode==0)
			RenderTriangle(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),c);
		if(Mode==1)
			RenderTriangleWire(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),c,1.0f);
		if(Mode==2){
			RenderTriangle(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),c);
			RenderTriangleWire(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),WHITE,1.0f);
		}
	}

	CStr_RenderAlxFontf(WINDOW_STD_ARGS,GetAlxFont(),0.0f,0.0f,RED,"L: %d",axis_line);
}
void Delete(AlxWindow* w){
	RubiksCube_Free(&cube);
	World3D_Free(&world);
	AlxWindow_Mouse_SetVisible(&window);
}

int main(){
	if(Create("Rubik's Cube",1900,1000,1,1,Setup,Update,Delete))
        Start();
    return 0;
}