#define GLUT_DISABLE_ATEXIT_HACK
#include <stdio.h>
#include <GL/glut.h>
#define pi 3.14
#include <math.h>
#include<stdbool.h>
#include<string.h>


static int counter = 0;
int animation_matrix_i = 0;
int turn = 0;
bool turn_complete = true;
const int color[][3] = { {1,0,0},{0,1,0},{0,0,1},{1,1,1},{1,1,0} }; //RGBW
float c_x, c_y;
//front end global variable section
int cols = 8;
int rows = 11;
static int count = 0;
static bool endgame = false;
int l_margin = -5;
int r_margin = 5;
int b_margin = -5;
int t_margin = 5;

float x_width;
float y_width;
float translate_factor = 0.7;
int wnd_scale = 65;

int arr_s[] = { -1,-1,-1,-1 }; //xmin,xmax,ymin,ymax
int screen_a[] = { -1,-1 }; // rowxcol

static int frame_buffer[11][8][6]; // 4 points
int frame_count = 0;

float ellipse_1[361][2]; // constant
float ellipse_2[361][2]; // constant
float circle[361][2];
float circle_move[361][2];
static int i_x = 0, i_y = 0; // used for circle revolution

static int max_speed_time = 0;
static int max_speed = 2;
bool mouse_click = false;
bool do_rotation = false;
static int vib_x;
static int vib_y;

static int translate_timer = 0;//used for cirlce translation
bool do_translation = false;
static float translate;
char move_to;
int recursive_array[88][4][3][2]; //0->from 1->to 2->excitation [3] -> from,to,to_excitation all 0's indicate cell is to be skipped
static int r_rows = 0;
//int r_cols = 0;
bool animateContinue = false;
bool doneAnimateStep = false;
static int animation_timer = 60;
static int animation_counter = 0;
bool startAnimation = false;
bool beginAnimation = false;
//backend global variable section
const int pos_x_travel[4] = { 0,0,-1,1 };//RLTB
const int pos_y_travel[4] = { 1,-1,0,0 };//RLTB
static int board[11][8][2];
int b_h = 11;
int b_l = 8;
int b_d = 2;

int movement_counter = 0;
int cell_counter = 0;

int movementBuffer[4][1][2];
bool Drawing = false;
static bool start_game = false;
static bool no_of_players_selected = false;
int recursive_array_2[88][4][3][2];
static int no_of_players = 3;

bool player_win = false;
bool can_click_mouse = true;
static int no_of_turns_played = 0;
static int player_id_count[8];
bool max_turns_played = false;
bool player_allowed_to_play[8] = { true,true,true,true,true,true };
bool frame_draw_complete = true;

char intro[] = "The objective of Chain reaction is to take the control@of the  board by eliminating your opponents orbs.@Players takes it in turns to place their orbs in a cell.@Once a cell has reached critical mass the orbs explode@into the surrounding cells adding an extra orb and@claiming the cells for the player. A player may only place@their own orbs in a blank cell or a cell that contains@orbs of their colour.As soon as a player loses all their@orbs they are out of the game.\n";
//end of global section


//backend functions


void array_to_screen(int x, int y)
{
	arr_s[0] = -1;
	arr_s[1] = -1;
	arr_s[2] = -1;
	arr_s[3] = -1;

	if (x >= 0 && x < rows && y >= 0 && y < cols)
	{

		arr_s[0] = (y * 5);
		arr_s[1] = (y * 5) + 5;
		arr_s[2] = -(x * 5);
		arr_s[3] = -(x * 5) - 5;
	}

}

void screen_to_array(int x, int y)
{
	screen_a[0] = -1;
	screen_a[1] = -1;
	int x_val = ((int)(x / x_width));
	int y_val = ((int)(y / y_width));
	if (x_val > 0 && x_val <= cols && y_val > 0 && y_val <= rows) //check if it lies within boundary
	{
		x_val = (x_val - 1) % cols; // sets x and y in terms of array indexes
		y_val = (y_val - 1) % rows;
		screen_a[1] = x_val;
		screen_a[0] = y_val;

	}

}


void generateEmptyPlane()
{
	for (int i = 0; i < b_h; i++)
	{
		for (int j = 0; j < b_l; j++)
		{
			board[i][j][0] = 0;
			board[i][j][1] = -1; //this is id

		}
	}
}

void displayGrid()
{
	for (int i = 0; i < b_h; i++)
	{
		for (int j = 0; j < b_l; j++)
		{
			printf("%d\t", board[i][j][0]);
		}
		printf("\n");
	}


}






int isCorner(int l, int m)
{

	int height = b_h - 1; // no of rows
	int length = b_l - 1; // no of columns
	if (l%height == 0 && m%length == 0)
	{
		//printf("\nIs Corner");
		return 1; // 1 indicates true
	}
	else
		return -2; // false
}

int isCornerColumnRow(int l, int m)
{
	int height = b_h - 1; // no of rows
	int length = b_l - 1; // no of columns
	if (m%length == 0)
		return 2;
	else if (l%height == 0)
		return 2;
	else
		return -2; //false

}

int getMaxExcitation(int l, int m) //This method generates the maximum excitation for a given cell based on its positon
{
	if (isCorner(l, m) != -2)
	{

		return isCorner(l, m);
	}
	else if (isCornerColumnRow(l, m) != -2)
		return isCornerColumnRow(l, m);
	else
		return 3;
}

void exciteNeighbours(int pos_x, int pos_y, int id) //this method will also be used to supply information to the animator
{
	//printf("\nFirst %d\t%d", pos_x, pos_y);
	board[pos_x][pos_y][0] = 0; //make the box stable
	board[pos_x][pos_y][1] = -1;//reset id
	//frame_buffer[pos_x][pos_y][0] = 0; //dont draw

	//frame_buffer[pos_x][pos_y][5] = -1;

	int r_rows_local = r_rows;
	int r_cols = 0;

	for (int i = 0; i < 4; i++)
	{
		if (pos_x + pos_x_travel[i] >= 0 && pos_x + pos_x_travel[i] < b_h) //ensure that control remains within the grid
		{
			if (pos_y + pos_y_travel[i] >= 0 && pos_y + pos_y_travel[i] < b_l) //ensure that control remains within the grid
			{

				//use the gui code to move the sphere for a valid neighbour position ie move from (pos_x,pos_y) to (pos_x+pos_x_travel[i],pos_y+pos_y_travel[i])
				board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][1] = id; //annex the neighbour
				//frame_buffer[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][5] = id; //annex the neighbour
				//frame_buffer[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0] = board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0];



				if (board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0] + 1 > getMaxExcitation(pos_x + pos_x_travel[i], pos_y + pos_y_travel[i]))
				{
					recursive_array[r_rows_local][r_cols][2][0] = board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0]; //to_excitation
					recursive_array_2[r_rows_local][r_cols][2][0] = board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0];
					recursive_array[r_rows_local][r_cols][2][1] = id;
					recursive_array_2[r_rows_local][r_cols][2][1] = id;
					recursive_array[r_rows_local][r_cols][0][0] = pos_x;//from
					recursive_array_2[r_rows_local][r_cols][0][0] = pos_x;//from
					recursive_array[r_rows_local][r_cols][0][1] = pos_y;//from
					recursive_array_2[r_rows_local][r_cols][0][1] = pos_y;//from
					recursive_array[r_rows_local][r_cols][1][0] = pos_x + pos_x_travel[i];//to
					recursive_array_2[r_rows_local][r_cols][1][0] = pos_x + pos_x_travel[i];//to
					recursive_array[r_rows_local][r_cols][1][1] = pos_y + pos_y_travel[i];//to
					recursive_array_2[r_rows_local][r_cols++][1][1] = pos_y + pos_y_travel[i];//to



					r_rows++; //increment recursive id for next iteration(static variable)
					exciteNeighbours(pos_x + pos_x_travel[i], pos_y + pos_y_travel[i], id); //use a recursive stack "THIS IS THE MAIN LOGIC OF CHAINREACTION"
				}
				else
				{

					board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0]++; //excite the neighbour cell
					//frame_buffer[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0] = board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0];
					recursive_array[r_rows_local][r_cols][2][0] = board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0]; //to_excitation
					recursive_array_2[r_rows_local][r_cols][2][0] = board[pos_x + pos_x_travel[i]][pos_y + pos_y_travel[i]][0];

					recursive_array[r_rows_local][r_cols][2][1] = id;
					recursive_array_2[r_rows_local][r_cols][2][1] = id;

					recursive_array[r_rows_local][r_cols][0][0] = pos_x;//from
					recursive_array_2[r_rows_local][r_cols][0][0] = pos_x;//from

					recursive_array[r_rows_local][r_cols][0][1] = pos_y;//from
					recursive_array_2[r_rows_local][r_cols][0][1] = pos_y;//from

					recursive_array[r_rows_local][r_cols][1][0] = pos_x + pos_x_travel[i];//to
					recursive_array_2[r_rows_local][r_cols][1][0] = pos_x + pos_x_travel[i];//to

					recursive_array[r_rows_local][r_cols][1][1] = pos_y + pos_y_travel[i];//to
					recursive_array_2[r_rows_local][r_cols++][1][1] = pos_y + pos_y_travel[i];//to



				}
			}
		}

	}

	return;


}


void AnimationMatrix()
{

	int i = animation_matrix_i;

	for (int j = 0; j < 4; j++)
	{

		if (!(recursive_array[i][j][0][0] == recursive_array[i][j][1][0] && recursive_array[i][j][0][1] == recursive_array[i][j][1][1]))
		{

			array_to_screen(recursive_array[i][j][0][0], recursive_array[i][j][0][1]); //source cell
			//printf("From %f\t%f\n", arr_s[0] + (arr_s[1] - arr_s[0]) / (float)2, arr_s[2] - (arr_s[2] - arr_s[3]) / (float)2);
			//printf("Rounded values From %d\t%d\n", (int)(arr_s[0] + (arr_s[1] - arr_s[0]) / (float)2), (int)(arr_s[2] - (arr_s[2] - arr_s[3]) / (float)2));
			recursive_array[i][j][0][0] = (int)(arr_s[0] + (arr_s[1] - arr_s[0]) / (float)2);
			recursive_array[i][j][0][1] = (int)(arr_s[2] - (arr_s[2] - arr_s[3]) / (float)2);

			array_to_screen(recursive_array[i][j][1][0], recursive_array[i][j][1][1]); //destination cell
			//printf("To %f\t%f\n", arr_s[0] + (arr_s[1] - arr_s[0]) / (float)2, arr_s[2] - (arr_s[2] - arr_s[3]) / (float)2);
			//printf("Rounded values From %d\t%d\n", (int)(arr_s[0] + (arr_s[1] - arr_s[0]) / (float)2), (int)(arr_s[2] - (arr_s[2] - arr_s[3]) / (float)2));
			recursive_array[i][j][1][0] = (int)(arr_s[0] + (arr_s[1] - arr_s[0]) / (float)2);
			recursive_array[i][j][1][1] = (int)(arr_s[2] - (arr_s[2] - arr_s[3]) / (float)2);


		}
	}
	animation_matrix_i++;




}

bool checkAliveStatus()
{
	turn_complete = true;
	int prev_turn = turn;
	if (max_turns_played && turn_complete)
	{

		for (int i = 0; i < b_h; i++)
		{
			for (int j = 0; j < b_l; j++)
				if (frame_buffer[i][j][0] > 0)
					player_id_count[frame_buffer[i][j][5]]++;
		}

		for (int i = 0; i < no_of_players; i++)
			if (player_id_count[i] == 0)
				player_allowed_to_play[i] = false;

		for (int i = 0; i < no_of_players; i++)
		{
			player_id_count[i] = 0;
		}

	}

	do {
		turn = (turn + 1) % no_of_players;
	} while (!player_allowed_to_play[turn]);

	if (prev_turn == turn)
	{
		printf("Game over \nPlayer %d wins \n", turn);
		endgame = true;
		return true;
	}
	return false;
}


void detectRule(int l, int m, int id) //array co-ordinates
{
	//printf("DetectRule\t%d\n", id);
	/*
	First check if the player has performed a legal action -> ie he has clicked on a tile that belongs to him
	use [][][1] to get player_id
	id with -1 indicates an empty tile ie non-claimed tile
	*/
	r_rows = 0;
	if (board[l][m][1] != id && board[l][m][1] != -1)  //allowed to click on empty tile or on his tile
	{

		return;
	}
	else
	{
		//r_rows = -1;
		//turn_complete = true; //click registered
		//turn = (turn + 1) % 2; //next players turn
		turn_complete = false;
		no_of_turns_played++;
		if (no_of_turns_played == no_of_players)
		{
			max_turns_played = true;


		}

		frame_buffer[l][m][5] = id;
		board[l][m][1] = id;
		if (board[l][m][0] + 1 > getMaxExcitation(l, m))
		{
			//	printf("In if part");
			frame_buffer[l][m][0] = board[l][m][0];

			exciteNeighbours(l, m, id);
			//AnimationMatrix();
			beginAnimation = true;


		}
		else
		{
			//printf("\nIn else block");
			board[l][m][0]++;
			frame_buffer[l][m][0] = board[l][m][0];
			//printf("Frame buffer value is %d\n", frame_buffer[l][m][0]);


			checkAliveStatus();


		}

		glutPostRedisplay();
	}

}









//front end functions





void computeCircle(float radius) //called once
{
	float radiance, x, y;

	for (int angle = 0; angle < 362; angle++)
	{
		radiance = (pi*angle) / (float)180;
		x = (radius)*cosf(radiance);
		y = (radius)*sinf(radiance);
		circle[angle][0] = x;
		circle[angle][1] = y;
	}
}

void computeCircle2(float radius) //called once
{
	float radiance, x, y;

	for (int angle = 0; angle < 362; angle++)
	{
		radiance = (pi*angle) / (float)180;
		x = (radius)*cosf(radiance);
		y = (radius)*sinf(radiance);
		circle_move[angle][0] = x;
		circle_move[angle][1] = y;
	}
}

void translateCircleMove(float h, float k)
{

	float x, y;
	for (int angle = 0; angle < 362; angle++)
	{

		x = circle_move[angle][0];
		y = circle_move[angle][1];

		glBegin(GL_LINES);
		glVertex2f(h, k);
		glVertex2f(h + x, k + y);
		glEnd();

	}


}

void translateCircle(float h, float k, int c)
{
	float x, y;

	for (int angle = 0; angle < 362; angle += 20)
	{

		x = circle[angle][0];
		y = circle[angle][1];

		if (c == 2)
		{
			glBegin(GL_LINES);
			glVertex2f(h + ellipse_1[i_x][0], k + ellipse_1[i_x][1]);
			glVertex2f(h + ellipse_1[i_x][0] + x, k + ellipse_1[i_x][1] + y);
			glEnd();

			glBegin(GL_LINES);
			glVertex2f(h, k);
			glVertex2f(h + x, k + y);
			glEnd();
		}

		if (c == 3)
		{
			glBegin(GL_LINES);
			glVertex2f(h + ellipse_2[i_y][0], k + ellipse_2[i_y][1]);
			glVertex2f(h + ellipse_2[i_y][0] + x, k + ellipse_2[i_y][1] + y);
			glEnd();

			glBegin(GL_LINES);
			glVertex2f(h + ellipse_1[i_x][0], k + ellipse_1[i_x][1]);
			glVertex2f(h + ellipse_1[i_x][0] + x, k + ellipse_1[i_x][1] + y);
			glEnd();

			glBegin(GL_LINES);
			glVertex2f(h, k);
			glVertex2f(h + x, k + y);
			glEnd();
		}

		if (c == 1)
		{
			glBegin(GL_LINES);
			glVertex2f(h, k);
			glVertex2f(h + x, k + y);
			glEnd();
		}

	}

}

void computeEllipse1(float h, float k, float a, float b) //h,k wrt to origin ie 0,0 //called once
{
	float radiance, x, y;
	for (int angle = 0; angle <= 360; angle++)
	{
		radiance = (pi*angle) / (float)180;
		x = h + (a*cosf(radiance));
		y = k + (b*sinf(radiance));
		ellipse_1[angle][0] = x;
		ellipse_1[angle][1] = y;
		glVertex2f(x, y);
	}
}

void computeEllipse2(float h, float k, float a, float b) //called once
{
	float radiance, x, y;
	for (int angle = 0; angle <= 360; angle++)
	{
		radiance = (pi*angle) / (float)180;
		x = h + (a*cosf(radiance));
		y = k + (b*sinf(radiance));
		ellipse_2[angle][0] = x;
		ellipse_2[angle][1] = y;
		glVertex2f(x, y);
	}
}

void draw(float h, float k, int c)// draw the 2 ellipse
{
	//if (c == 3)
//	{
	//	glBegin(GL_POINTS);
	//	for (int i = 0;i < 362;i+=2)
		//{
	//
		//	glVertex2f(h + ellipse_1[i][0], k + ellipse_1[i][1]);
		//	glVertex2f(h + ellipse_2[i][0], k + ellipse_2[i][1]);
	//	}
	//	glEnd();
	//}

	//if (c == 2)
	//{
	//	glBegin(GL_POINTS);
	//	for (int i = 0;i < 362;i++)
	//	{
//
	//		glVertex2f(h + ellipse_1[i][0], k + ellipse_1[i][1]);
	//		
	//	}
	//	glEnd();
//	}
	//translateCircle(h, k, c);

	translateCircle(h, k, 1);
	//check if the cell clicked is corner


	if (c == 2)
	{
		int index = 0;
		glBegin(GL_POINTS);
		for (int i = 40; i >= 0; i--)
		{
			if (i_x >= 40)
				index = i_x - i;
			else
				index = i_x;

			glVertex2f(h + ellipse_1[index][0], k + ellipse_1[index][1]);
		}
		glEnd();
	}

	if (c == 3)
	{
		int index_x = 0;
		int index_y = 0;

		glBegin(GL_POINTS);

		for (int i = 50; i >= 0; i--)
		{
			if (i_x >= 50)
				index_x = i_x - i;
			else
				index_x = i_x;

			if (i_y >= 50)
				index_y = i_y - i;
			else
				index_y = i_y;

			glVertex2f(h + ellipse_1[index_x][0], k + ellipse_1[index_x][1]);
			glVertex2f(h + ellipse_2[index_y][0], k + ellipse_2[index_y][1]);
		}

		glEnd();
	}
}




void frame_draw()
{

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			int c = 0;
			if (isCorner(i, j) == 1)
				c = 3;
			else if (isCornerColumnRow(i, j) == 2 && frame_buffer[i][j][0] == 2)
				c = 3;
			else if (isCornerColumnRow(i, j) == 2 && frame_buffer[i][j][0] == 1)
				c = 2;
			else
				c = frame_buffer[i][j][0];

			if (frame_buffer[i][j][0] > 0)
			{

				glColor3f(color[frame_buffer[i][j][5]][0], color[frame_buffer[i][j][5]][1], color[frame_buffer[i][j][5]][2]);

				draw(frame_buffer[i][j][1] + (frame_buffer[i][j][2] - frame_buffer[i][j][1]) / (float)2, frame_buffer[i][j][3] - (frame_buffer[i][j][3] - frame_buffer[i][j][4]) / (float)2, c); //draw ellipse first
				//translateCircle(frame_buffer[i][j][1] + (frame_buffer[i][j][2] - frame_buffer[i][j][1]) / (float)2, frame_buffer[i][j][3] - (frame_buffer[i][j][3] - frame_buffer[i][j][4]) / (float)2,frame_buffer[i][j][0]); // draw the circles


			}
		}

	}

	//glFlush();
}







void fill_frame_buffer() //call this only once
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			array_to_screen(i, j);

			frame_buffer[i][j][0] = 0; //default excitation
			frame_buffer[i][j][1] = arr_s[0];
			frame_buffer[i][j][2] = arr_s[1];
			frame_buffer[i][j][3] = arr_s[2];
			frame_buffer[i][j][4] = arr_s[3];
			frame_buffer[i][j][5] = -1; //default player id

		}
	}
}



void animate()
{


	for (int j = 0; j < 4; j++)
	{

		if (!(recursive_array[cell_counter][j][0][0] == recursive_array[cell_counter][j][1][0] && recursive_array[cell_counter][j][0][1] == recursive_array[cell_counter][j][1][1]))
		{
			//create a circle and move it


			if (recursive_array[cell_counter][j][0][0] == recursive_array[cell_counter][j][1][0])
			{

				int sign = recursive_array[cell_counter][j][0][1] > recursive_array[cell_counter][j][1][1] ? -1 : 1;

				if (recursive_array[cell_counter][j][0][1] != recursive_array[cell_counter][j][1][1])
				{

					translateCircleMove(recursive_array[cell_counter][j][0][0], recursive_array[cell_counter][j][0][1] + 1 * sign); //draw a basic circle
					movementBuffer[j][0][0] = recursive_array[cell_counter][j][0][0];
					movementBuffer[j][0][1] = recursive_array[cell_counter][j][0][1] + 1 * sign;

					recursive_array[cell_counter][j][0][1] += (1 * sign);

				}


			}
			else
			{

				int sign = recursive_array[cell_counter][j][0][0] > recursive_array[cell_counter][j][1][0] ? -1 : 1;
				if (recursive_array[cell_counter][j][0][0] != recursive_array[cell_counter][j][1][0]) {
					translateCircleMove(recursive_array[cell_counter][j][0][0] + 1 * sign, recursive_array[cell_counter][j][0][1]); //draw a basic circle
					movementBuffer[j][0][0] = recursive_array[cell_counter][j][0][0] + 1 * sign;
					movementBuffer[j][0][1] = recursive_array[cell_counter][j][0][1];

					recursive_array[cell_counter][j][0][0] += (1 * sign);

				}


			}

		}


	}

	Drawing = true;




}

void mouse(int button, int state, int x, int y)
{
	if (no_of_players_selected && !can_click_mouse)
	{

		screen_to_array(x, y);

		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		{
			if (screen_a[0] == 5 && (screen_a[1] == 3 || screen_a[1] == 4))
			{
				start_game = true;
				can_click_mouse = true;
				glutDestroyMenu(glutGetMenu());
				glutPostRedisplay();
				return;
			}
		}
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && can_click_mouse)
	{
		screen_to_array(x, y);


		if (screen_a[0] != -1 && screen_a[1] != -1) //if the click is valid
		{


			do_rotation = true;


			detectRule(screen_a[0], screen_a[1], turn);


		}

	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		printf("Recurisive id\tFrom\t\tTo\t\tID\tExcitation\n");

		//animate();
		//startAnimation = true;
		for (int i = 0; i <= r_rows; i++)
		{
			for (int j = 0; j < 4; j++)
				printf("%d\t\t%d\t%d\t%d\t%d\t%d\t%d\n", i, recursive_array_2[i][j][0][0], recursive_array_2[i][j][0][1], recursive_array_2[i][j][1][0], recursive_array_2[i][j][1][1], recursive_array_2[i][j][2][1], recursive_array_2[i][j][2][0]);
		}

		printf("\n");
		for (int i = 0; i < no_of_players; i++)
		{
			if (player_allowed_to_play[i])
				printf("player %d is %s\t", i, "True");
		}
		printf("\n");
	}

}

void drawBitmapText(char data[], float x, float y, float z)
{
	char c;
	int i;
	glRasterPos3f(x, y, z);
	for (i = 0; i < strlen(data); i++)
	{
		c = data[i];
		if (c == '@')
		{
			y -= 2;
			glRasterPos3f(x, y, z);
			continue;
		}

		if (c == '%')
			c = '1' + turn;
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}

}
void player_menu(int c)
{
	no_of_players_selected = true;
	no_of_players = c;


}

void endGame()
{

	draw(c_x, c_y, 3);


}

void display(void) {


	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (endgame)
	{
		char arrwin[] = "Player % wins";
		endGame();
		drawBitmapText(arrwin, c_x - 8, c_y + 10, 0);
		glFlush();


	}

	else
	{
		if (!start_game)
		{
			char arr[] = "Play game";
			glColor3f(0.1, 0.1, 0.1);
			glBegin(GL_POLYGON);
			glVertex2f(c_x - (l_margin * 2 / (float)2), c_y + (t_margin / (float)2)); //top left
			glVertex2f(c_x + (l_margin * 2 / (float)2), c_y + (t_margin / (float)2)); //top right
			glVertex2f(c_x + (l_margin * 2 / (float)2), c_y - (t_margin / (float)2)); //bottom right
			glVertex2f(c_x - (l_margin * 2 / (float)2), c_y - (t_margin / (float)2)); //top left
			glEnd();
			glColor3f(1, 1, 1);
			drawBitmapText(arr, c_x - 4, c_y - 0.5, 0);
			glColor3f(1, 1, 1);
			drawBitmapText(intro, -2, c_y - 10, 0);

			glFlush();
		}

		if (start_game && no_of_players_selected)
		{
			if (!player_win)
			{
				glColor3f(color[turn][0], color[turn][1], color[turn][2]);

				for (int i = 0; i < cols + 1; i++)
				{
					glBegin(GL_LINES);
					glVertex2i(i * 5, 0);
					glVertex2i(i * 5, -rows * 5);
					glEnd();

				}
				for (int i = 0; i < rows + 1; i++)
				{
					glBegin(GL_LINES);
					glVertex2i(0, -i * 5);
					glVertex2i(cols * 5, -i * 5);
					glEnd();
				}

				glPushMatrix();
				glTranslatef(translate_factor, translate_factor, 0);

				glBegin(GL_LINES);
				for (int i = 0; i < cols + 1; i++)
				{
					glVertex2i(i * 5, 0);
					glVertex2i(i * 5, -rows * 5);
				}
				for (int i = 0; i < rows + 1; i++)
				{
					glVertex2i(0, -i * 5);
					glVertex2i(cols * 5, -i * 5);
				}
				glEnd();
				glPopMatrix();

				for (int j = 0; j < rows + 1; j++)
				{
					for (int i = 0; i < cols + 1; i++)
					{
						glBegin(GL_LINES);
						glVertex2f(i * 5, j*(-5));
						glVertex2f(i * 5 + (translate_factor), (j*-5) + (translate_factor));
						glEnd();
					}
				}




				frame_draw();
				glColor3f(color[turn][0], color[turn][1], color[turn][2]);

				if (Drawing)
				{

					for (int i = 0; i < 4; i++)
					{
						if (movementBuffer[i][0][0] != 0 && movementBuffer[i][0][1] != 0)
						{
							translateCircleMove(movementBuffer[i][0][0], movementBuffer[i][0][1]);
						}
					}
				}

				glFlush();

			}
			else
			{
				printf("Player wins\n");

				glutPostRedisplay();
			}
		}

	}
}

void init() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0 + l_margin, cols * 5 + r_margin, -(rows * 5) + b_margin, 0 + t_margin);
	glMatrixMode(GL_MODELVIEW);


}


void idle() {





	max_speed_time = (max_speed_time + 1) % (max_speed + 1);
	animation_counter = (animation_counter + 1) % (animation_timer + 1);
	if (do_rotation)
	{
		if (max_speed_time == max_speed && !mouse_click)
		{

			i_x = (i_x + 1) % 362;
			if (i_y >= 1)
				i_y = (i_y - 1) % 362;
			else
				i_y = 360;
			glutPostRedisplay();

		}

	}

	if (animation_counter == animation_timer && beginAnimation) //call every some ms
	{
		//move 4 balls by 1 unit in their respective direction

		//call animate 5 times

		if (counter == 0)
		{
			frame_buffer[recursive_array_2[cell_counter][0][0][0]][recursive_array_2[cell_counter][0][0][1]][5] = -1; //id
			frame_buffer[recursive_array_2[cell_counter][0][0][0]][recursive_array_2[cell_counter][0][0][1]][0] = 0; //excitation
			AnimationMatrix();
			can_click_mouse = false;

		}

		counter++;

		if (counter < 5)
		{
			Drawing = false;
			animate();
			glutPostRedisplay();
		}

		else
		{
			//update framebuffer
			Drawing = false;
			int i = cell_counter;

			for (int j = 0; j < 4; j++)
				if (!(recursive_array[i][j][0][0] == recursive_array[i][j][1][0] && recursive_array[i][j][0][1] == recursive_array[i][j][1][1]))
				{

					frame_buffer[recursive_array_2[i][j][1][0]][recursive_array_2[i][j][1][1]][5] = recursive_array_2[i][j][2][1];
					frame_buffer[recursive_array_2[i][j][1][0]][recursive_array_2[i][j][1][1]][0] = recursive_array_2[i][j][2][0];

				}



			if (cell_counter < r_rows)
			{


				counter = 0;
				cell_counter++;



			}

			else
			{
				animation_matrix_i = 0;
				cell_counter = 0;
				counter = 0;

				can_click_mouse = true;
				if (checkAliveStatus())
					printf("\nGame over");
				beginAnimation = false;
				Drawing = false;
			}
		}
	}

	glutPostRedisplay();
	return;
}

int main(int argc, char **argv) {

	can_click_mouse = false;
	computeEllipse1(0, 0, 2, 1);
	computeEllipse2(0, 0, 1, 2);
	computeCircle(0.3);
	computeCircle2(0.5); //tune this parameter if needed
	fill_frame_buffer();
	generateEmptyPlane();
	x_width = (cols*wnd_scale) / (cols + 2);
	y_width = (rows*wnd_scale) / (rows + 2);
	c_x = (cols / 2) * 5;
	c_y = -(rows / 2.0) * 5;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(cols*wnd_scale, rows*wnd_scale);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - (cols*wnd_scale)) / 2,
		(glutGet(GLUT_SCREEN_HEIGHT) - (rows*wnd_scale)) / 2);
	glutCreateWindow("Chain Reaction");
	init();
	glutMouseFunc(mouse);
	glutDisplayFunc(display);
	glutCreateMenu(player_menu);
	glutAddMenuEntry("2", 2);
	glutAddMenuEntry("3", 3);
	glutAddMenuEntry("4", 4);
	glutAddMenuEntry("5", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutIdleFunc(idle);
	glutMainLoop();
}