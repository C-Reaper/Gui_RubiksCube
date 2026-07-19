#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Cube.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Mathlib.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Mesh.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_World3D.h"


// AXIS Count fixed to dimensions -> 3D
// axis_piece = [ 0,FIELD_COUNT [

typedef enum {
	RUBIKSCUBE_FIELD_NONE 		= 0U,
	RUBIKSCUBE_FIELD_0 			= 1U,
	RUBIKSCUBE_FIELD_1 			= 2U,
	RUBIKSCUBE_FIELD_2 			= 3U,
	RUBIKSCUBE_FIELD_3 			= 4U,
	RUBIKSCUBE_FIELD_4 			= 5U,
	RUBIKSCUBE_FIELD_5 			= 6U
} RubiksCube_Field;

typedef enum {
	RUBIKSCUBE_SIDE_FIELDCOUNT  = 3U,
	RUBIKSCUBE_SIDE_FIELDCOUNT2 = RUBIKSCUBE_SIDE_FIELDCOUNT * RUBIKSCUBE_SIDE_FIELDCOUNT,
	RUBIKSCUBE_SIDE_COUNT  		= 6U,
	RUBIKSCUBE_SIDE_FRONT  		= 0U,
	RUBIKSCUBE_SIDE_LEFT   		= 1U,
	RUBIKSCUBE_SIDE_BACK   		= 2U,
	RUBIKSCUBE_SIDE_RIGHT  		= 3U,
	RUBIKSCUBE_SIDE_TOP    		= 4U,
	RUBIKSCUBE_SIDE_BOTTOM 		= 5U,
} RubiksCube_Side;

float RubiksCube_Side_Rotations[RUBIKSCUBE_SIDE_COUNT][2] = {
	{ 0.0f,0.0f },
	{ 0.0f,F32_PI05 },
	{ 0.0f,F32_PI },
	{ 0.0f,F32_PI05 * 3 },
	{ F32_PI05 * 3,0.0f },
	{ F32_PI05,0.0f }
};
const int RubiksCube_Side_Dir[] = {
   -1,
    1
};
unsigned char RubiksCube_Side_AxisDir[3] = {
	1U,0U,0U
};
RubiksCube_Side RubiksCube_Side_AxisToSide[3][2] = {
	{ RUBIKSCUBE_SIDE_RIGHT,RUBIKSCUBE_SIDE_LEFT },
	{ RUBIKSCUBE_SIDE_TOP,RUBIKSCUBE_SIDE_BOTTOM },
	{ RUBIKSCUBE_SIDE_FRONT,RUBIKSCUBE_SIDE_BACK }
};
RubiksCube_Side RubiksCube_Side_AxisIndexToSide[3][4] = {
	{ RUBIKSCUBE_SIDE_FRONT,RUBIKSCUBE_SIDE_TOP,RUBIKSCUBE_SIDE_BACK,RUBIKSCUBE_SIDE_BOTTOM },
	{ RUBIKSCUBE_SIDE_FRONT,RUBIKSCUBE_SIDE_LEFT,RUBIKSCUBE_SIDE_BACK,RUBIKSCUBE_SIDE_RIGHT },
	{ RUBIKSCUBE_SIDE_LEFT,RUBIKSCUBE_SIDE_TOP,RUBIKSCUBE_SIDE_RIGHT,RUBIKSCUBE_SIDE_BOTTOM }
};

typedef struct RubiksCube_State {
    RubiksCube_Field sides[RUBIKSCUBE_SIDE_COUNT][RUBIKSCUBE_SIDE_FIELDCOUNT2];
} RubiksCube_State;

typedef struct RubiksCube_Move {
    unsigned char axis;
	unsigned char axis_line;
	unsigned char dir;
} RubiksCube_Move;

typedef struct RubiksCube{
	RubiksCube_Field sides[RUBIKSCUBE_SIDE_COUNT][RUBIKSCUBE_SIDE_FIELDCOUNT2];
	Pixel colors[RUBIKSCUBE_SIDE_COUNT];
	Vec3D pos;
	float ax;
	float ay;
	Vector trace;
} RubiksCube;

void RubiksCube_Reset(RubiksCube_State* c){
	for(int i = 0;i<RUBIKSCUBE_SIDE_COUNT;i++){
		for(int j = 0;j<RUBIKSCUBE_SIDE_FIELDCOUNT2;j++){
			c->sides[i][j] = i + 1U;
		}
	}
}
char RubiksCube_IsSolved(const RubiksCube_State* c) {
    for(int s = 0; s < RUBIKSCUBE_SIDE_COUNT; s++) {
        for(int i = 0; i < RUBIKSCUBE_SIDE_FIELDCOUNT2; i++) {
            if(c->sides[s][i] != (unsigned char)(s + 1U)) {
                return 0;
            }
        }
    }
    return 1;
}
void RubiksCube_CopyState(RubiksCube_State* dst, const RubiksCube_State* src) {
    memcpy(dst->sides, src->sides, sizeof(src->sides));
}
char RubiksCube_EqualState(const RubiksCube_State* a, const RubiksCube* b) {
    return memcmp(a->sides, b->sides, sizeof(a->sides)) == 0;
}

RubiksCube RubiksCube_New(Vec3D p,Pixel colors[RUBIKSCUBE_SIDE_COUNT]){
	RubiksCube c;
	c.pos = p;
	c.ax = 0.0f;
	c.ay = 0.0f;
	c.trace = Vector_New(sizeof(RubiksCube_Move));
	memcpy(c.colors,colors,sizeof(c.colors));
	RubiksCube_Reset((RubiksCube_State*)&c);
	return c;
}
void RubiksCube_Free(RubiksCube* c){
	Vector_Free(&c->trace);
}
int RubiksCube_MapIndex(int ixy,unsigned char toby){
	if(!toby){
		const int out = ixy - RUBIKSCUBE_SIDE_FIELDCOUNT / 2;
		return out + (out >= 0 && RUBIKSCUBE_SIDE_FIELDCOUNT % 2 == 0 ? 1 : 0);
	}else{
		return ixy + (ixy >= 0 && RUBIKSCUBE_SIDE_FIELDCOUNT % 2 == 0 ? -1 : 0) + RUBIKSCUBE_SIDE_FIELDCOUNT / 2;
	}
}
int RubiksCube_Mirror(unsigned char xy,int i){
	const int ix = i % RUBIKSCUBE_SIDE_FIELDCOUNT;
	const int iy = i / RUBIKSCUBE_SIDE_FIELDCOUNT;

	const int cx = ix - RUBIKSCUBE_SIDE_FIELDCOUNT / 2;
	const int cy = iy - RUBIKSCUBE_SIDE_FIELDCOUNT / 2;

	const int rtx = cx * (xy ? -1 : 1) + RUBIKSCUBE_SIDE_FIELDCOUNT / 2;
	const int rty = cy * (xy ? 1 : -1) + RUBIKSCUBE_SIDE_FIELDCOUNT / 2;

	return rty * RUBIKSCUBE_SIDE_FIELDCOUNT + rtx;
}
int RubiksCube_MirrorSide_Index(int i){
	const int ix = i % RUBIKSCUBE_SIDE_FIELDCOUNT;
	const int iy = i / RUBIKSCUBE_SIDE_FIELDCOUNT;

	const int cx = RubiksCube_MapIndex(ix,0U);
	const int cy = RubiksCube_MapIndex(iy,0U);

	const int tx = -cx;
	const int ty = -cy;

	const int rtx = RubiksCube_MapIndex(tx,1U);
	const int rty = RubiksCube_MapIndex(ty,1U);
	return rty * RUBIKSCUBE_SIDE_FIELDCOUNT + rtx;
}
int RubiksCube_RotateSide_Index(int i,unsigned char dir){
	const int ix = i % RUBIKSCUBE_SIDE_FIELDCOUNT;
	const int iy = i / RUBIKSCUBE_SIDE_FIELDCOUNT;

	const int cx = RubiksCube_MapIndex(ix,0U);
	const int cy = RubiksCube_MapIndex(iy,0U);

	const int tx = cy * RubiksCube_Side_Dir[dir];
	const int ty = cx * RubiksCube_Side_Dir[(dir + 1U) % 2U];

	const int rtx = RubiksCube_MapIndex(tx,1U);
	const int rty = RubiksCube_MapIndex(ty,1U);
	return rty * RUBIKSCUBE_SIDE_FIELDCOUNT + rtx;
}
void RubiksCube_RotateSide(RubiksCube* c,RubiksCube_Side side,unsigned char dir){
	RubiksCube_Field fields[RUBIKSCUBE_SIDE_FIELDCOUNT2];
	
	for(int i = 0;i<RUBIKSCUBE_SIDE_FIELDCOUNT2;i++){
		const int t_i = RubiksCube_RotateSide_Index(i,dir);
		fields[t_i] = c->sides[side][i];
	}

	memcpy(c->sides[side],fields,sizeof(fields));
}
int RubiksCube_Map(RubiksCube_Side side,unsigned char axis,int line,int index){
	const int i_m = (RubiksCube_Side_AxisDir[axis] ? RUBIKSCUBE_SIDE_FIELDCOUNT : 1);
	const int l_m = (RubiksCube_Side_AxisDir[axis] ? 1 : RUBIKSCUBE_SIDE_FIELDCOUNT);
	int out = index * i_m + line * l_m;

	if(axis == 0U){
		if(side == RUBIKSCUBE_SIDE_BACK){
			out = RubiksCube_MirrorSide_Index(out);
		}
	}else if(axis == 2U){
		if(side == RUBIKSCUBE_SIDE_LEFT){
			out = RubiksCube_RotateSide_Index(out,1U);
		}else if(side == RUBIKSCUBE_SIDE_RIGHT){
			out = RubiksCube_RotateSide_Index(out,0U);
		}else if(side == RUBIKSCUBE_SIDE_TOP){
			out = RubiksCube_MirrorSide_Index(out);
		}
	}

	return out;
}
void RubiksCube_RotateLine(RubiksCube* c,unsigned char axis,unsigned char axis_line,unsigned char dir){
	RubiksCube_Field fields[4][RUBIKSCUBE_SIDE_FIELDCOUNT];
	
	for(int i = 0;i<4;i++){
		const int t_i = (i + (dir ? 1 : -1) + 4) % 4;

		const RubiksCube_Side i_s = RubiksCube_Side_AxisIndexToSide[axis][i];
		const RubiksCube_Side t_s = RubiksCube_Side_AxisIndexToSide[axis][t_i];

		for(int j = 0;j<RUBIKSCUBE_SIDE_FIELDCOUNT;j++){
			const int i_j = RubiksCube_Map(i_s,axis,axis_line,j);
			fields[i][j] = c->sides[i_s][i_j];
		}
	}
	for(int i = 0;i<4;i++){
		const int t_i = (i + (dir ? 1 : -1) + 4) % 4;

		const RubiksCube_Side i_s = RubiksCube_Side_AxisIndexToSide[axis][i];
		const RubiksCube_Side t_s = RubiksCube_Side_AxisIndexToSide[axis][t_i];
		
		for(int j = 0;j<RUBIKSCUBE_SIDE_FIELDCOUNT;j++){
			const int i_j = RubiksCube_Map(t_s,axis,axis_line,j);
			c->sides[t_s][i_j] = fields[i][j];
		}
	}
}
void RubiksCube_Rotate(RubiksCube* c,unsigned char axis,unsigned char axis_line,unsigned char dir){
	if(axis_line == 0U)
		RubiksCube_RotateSide(c,RubiksCube_Side_AxisToSide[axis][0],dir);
	else if(axis_line + 1U == RUBIKSCUBE_SIDE_FIELDCOUNT)
		RubiksCube_RotateSide(c,RubiksCube_Side_AxisToSide[axis][1],(dir + 1U) % 2U);
	
	RubiksCube_RotateLine(c,axis,axis_line,dir);
}
void RubiksCube_Shuffle(RubiksCube* c,unsigned int count){
	for(unsigned int i = 0;i<count;i++){
		const unsigned char axis = Random_u64_MinMax(0U,3U);
		const unsigned char axis_line = Random_u64_MinMax(0U,RUBIKSCUBE_SIDE_FIELDCOUNT);
		const unsigned char dir = Random_u64_MinMax(0U,2U);

		Vector_Push(&c->trace,(RubiksCube_Move[]){{
			.axis = axis,
			.axis_line = axis_line,
			.dir = dir
		}});

		RubiksCube_Rotate(
			c,
			axis,
			axis_line,
			dir
		);
	}
}
void RubiksCube_Unshuffle(RubiksCube* c,unsigned int count){
	for(unsigned int i = 0;i<count && c->trace.size > 0;i++){
		const RubiksCube_Move* const rm = (RubiksCube_Move*)Vector_Get(&c->trace,c->trace.size - 1);

		RubiksCube_Rotate(
			c,
			rm->axis,
			rm->axis_line,
			!rm->dir
		);
		
		Vector_PopTop(&c->trace);
	}
}

char RubiksCube_Solve_R(RubiksCube* c, RubiksCube_State* target, int depth, int max_depth) {
    if (depth > max_depth)
		return 0;
    if (RubiksCube_IsSolved((RubiksCube_State*)c))
		return 1;

    for (unsigned char axis = 0; axis < 3; axis++) {
        for (unsigned char line = 0; line < RUBIKSCUBE_SIDE_FIELDCOUNT; line++) {
            for (unsigned char dir = 0; dir < 2; dir++) {
                RubiksCube_Rotate(c, axis, line, dir);

                if (RubiksCube_Solve_R(c, target, depth + 1, max_depth)) {
                    return 1;
                }

                RubiksCube_Rotate(c, axis, line, (dir + 1) % 2);
            }
        }
    }
    return 0;
}
void RubiksCube_Solve(RubiksCube* c) {
    if (RubiksCube_IsSolved((RubiksCube_State*)c)) return;

	RubiksCube_State target;
    RubiksCube_Reset(&target);

    RubiksCube_State start;
    RubiksCube_CopyState(&start,(RubiksCube_State*)c);

    printf("[RubiksCube] Starting search from scrambled state...\n");

    bool solved = false;
    for (int max_d = 1; max_d <= 25 && !solved; max_d++) {
        printf("  Trying depth %d...\n",max_d);

        RubiksCube_CopyState((RubiksCube_State*)c,&start);

        if (RubiksCube_Solve_R(c,&target,0,max_d)) {
            printf("[RubiksCube] Solved in <= %d moves!\n",max_d);
            solved = true;
        }
    }

    if (!solved) {
        printf("[RubiksCube] Search limit reached -> Resetting.\n");
        RubiksCube_Reset((RubiksCube_State*)c);
    }
}
void RubiksCube_Render(RubiksCube* c,Vector* tris){
	const float pz = -((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f);
	const M4x4D cube_matX = Matrix_MakeRotationX(c->ax);
	const M4x4D cube_matY = Matrix_MakeRotationY(c->ay);
	const M4x4D cube_mat = Matrix_MultiplyMatrix(cube_matY,cube_matX);

	for(int i = 0;i<RUBIKSCUBE_SIDE_COUNT;i++){
		const M4x4D matX = Matrix_MakeRotationX(RubiksCube_Side_Rotations[i][0]);
		const M4x4D matY = Matrix_MakeRotationY(RubiksCube_Side_Rotations[i][1]);
		const M4x4D mat = Matrix_MultiplyMatrix(matY,matX);

		for(int j = 0;j<RUBIKSCUBE_SIDE_FIELDCOUNT2;j++){
			const int ix = j % RUBIKSCUBE_SIDE_FIELDCOUNT;
			const int iy = j / RUBIKSCUBE_SIDE_FIELDCOUNT;
			const RubiksCube_Field field = c->sides[i][j];

			if(field == 0U || field > RUBIKSCUBE_SIDE_COUNT) continue;

			const Vec3D tile_pos00 = Vec3D_New(
				(float)ix - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f),
				(float)iy - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f),
				pz
			);
			const Vec3D tile_pos10 = Vec3D_New(
				(float)ix - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f) + 1.0f,
				(float)iy - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f),
				pz
			);
			const Vec3D tile_pos01 = Vec3D_New(
				(float)ix - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f),
				(float)iy - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f) + 1.0f,
				pz
			);
			const Vec3D tile_pos11 = Vec3D_New(
				(float)ix - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f) + 1.0f,
				(float)iy - ((float)RUBIKSCUBE_SIDE_FIELDCOUNT * 0.5f) + 1.0f,
				pz
			);

			const Vec3D ord_pos00 = Matrix_MultiplyVector(mat,tile_pos00);
			const Vec3D ord_pos10 = Matrix_MultiplyVector(mat,tile_pos10);
			const Vec3D ord_pos01 = Matrix_MultiplyVector(mat,tile_pos01);
			const Vec3D ord_pos11 = Matrix_MultiplyVector(mat,tile_pos11);

			const Vec3D rot_pos00 = Matrix_MultiplyVector(cube_mat,ord_pos00);
			const Vec3D rot_pos10 = Matrix_MultiplyVector(cube_mat,ord_pos10);
			const Vec3D rot_pos01 = Matrix_MultiplyVector(cube_mat,ord_pos01);
			const Vec3D rot_pos11 = Matrix_MultiplyVector(cube_mat,ord_pos11);

			const M4x4D matX = Matrix_MakeRotationX(RubiksCube_Side_Rotations[i][0]);
			const M4x4D matY = Matrix_MakeRotationY(RubiksCube_Side_Rotations[i][1]);
			const M4x4D mat = Matrix_MultiplyMatrix(matY,matX);

			const Vec3D off_pos00 = Vec3D_Add(c->pos,rot_pos00);
			const Vec3D off_pos10 = Vec3D_Add(c->pos,rot_pos10);
			const Vec3D off_pos01 = Vec3D_Add(c->pos,rot_pos01);
			const Vec3D off_pos11 = Vec3D_Add(c->pos,rot_pos11);

			Tri3D tri0 = {
				.p = {
					off_pos00,
					off_pos11,
					off_pos10
				},
				.c = {
					.c = c->colors[field - 1U],
					.l = 1.0f
				}
			};
			Tri3D_CalcNorm(&tri0);
			Vector_Push(tris,&tri0);

			Tri3D tri1 = {
				.p = {
					off_pos00,
					off_pos01,
					off_pos11
				},
				.c = {
					.c = c->colors[field - 1U],
					.l = 1.0f
				}
			};
			Tri3D_CalcNorm(&tri1);
			Vector_Push(tris,&tri1);
		}
	}
}


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