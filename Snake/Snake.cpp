#include <easyx.h>      // 引用图形库头文件
#include <conio.h>
#include <time.h>
#include <stdio.h>

#define COL 20							// 游戏主界面的格子列数
#define ROW 20							// 游戏主界面的格子行数
#define GRID_WIDTH 20					// 格子的长宽
#define BORDER 4						// 面板分隔宽度
#define PADDING 10						// 控制面板补白
#define CONTROL_WIDTH 250				// 控制面板宽度
#define BOX_HEIGHT 30					
#define SNAKE_MAX_LEN 3072				// 蛇身结点最大长度
#define SPEED 300						// 速度（毫秒）
#define SPEED_INCREMENT 30				// 增速
#define FOOD_TIME 15					// 食物消失时间（秒）
#define SCORE 10						// 分数
#define BK_COLOR RGB(188, 230, 144)		// #bce672 松花色
#define SNAKE_COLOR RGB(00, 52, 114)	// #003472 花青色
#define FOOD_COLOR RGB(255, 45, 81)		// #ff2d51 火红色
#define BORDER_COLOR RGB(87, 66, 102)	// #574266 黛紫

// 方向
typedef enum Direction
{
	up,			// 0
	right,		// 1
	down,		// 2
	left		// 3
} Direction;

// 游戏状态
typedef enum Status
{
	starting,	// 0
	running,		// 1
	pause,		// 2
	end			// 3
} Status;

// 坐标
typedef struct Coordinate
{
	int top;
	int left;
} Coordinate;

// 游戏设置
struct Game
{
	Status status;			// 游戏状态
	int score;				// 分数
	int throutgh_the_wall;	// 是否可以穿墙？
	int width;				// 游戏面板宽度
	int height;				// 游戏面板高度
	HWND hWnd;				// 窗口句柄
} game;

// 主游戏面板

struct Game_Panel
{
	int col;				// 网格列数
	int row;				// 网格行数
	int grid_width;			// 网格宽度
	int width;				// 像素宽度
	int height;				// 像素高度
} gp;

// 控制面板
struct Control_Panel
{
	int width;
	int box_left;
	int box_right;
	int box_height;			// 每一行字所占盒子的高度
} cp;

// 蛇
struct Snake
{
	Coordinate body[3072];
	int length;				// 蛇的实际长度
	Direction direct;		// 蛇的方向
} snake;

//食物
struct Food
{
	Coordinate pos;
	time_t start;			// 建立时间
	int score;				// 分数
	int eated;				// 是否被吃，0表示未吃，1表示已吃
} food;

// 函数声明
//**********************************************************//
void init();				
void create_food();			// 创建食物
void draw();				// 绘制图形：网格、蛇、食物
void move();
void detect_key();			// 检测按键
void check();
void gameover();
//**********************************************************//

int main()
{
	init();
	
	BeginBatchDraw();
	draw();
	FlushBatchDraw();

	// 主循环
	while (1)
	{
		if (game.status == running) 
		{
			detect_key();
			move();
			check();
			draw();
			FlushBatchDraw();
			Sleep(SPEED);
			cleardevice();
		}
		else
		{
			detect_key();
			Sleep(SPEED);
		}
		
	}

	EndBatchDraw();
	closegraph();          // 关闭绘图窗口
}

void detect_key()
{
	int key;
	int messageID;
	if (_kbhit())
	{
		key = _getch();
		switch (key)
		{
		case 27:
			messageID = MessageBox(game.hWnd, _T("确定结束游戏？"), _T("结束游戏"), MB_OKCANCEL | MB_ICONWARNING);
			if (messageID == IDOK)
			{
				closegraph();
				exit(1);
			}
			break;
		case ' ':
			if (game.status == running) game.status = pause;
			else game.status = running;
			break;
		case 'w':
		case 'W':
			if (snake.direct != down)
				snake.direct = up;
			break;
		case 'd':
		case 'D':
			if (snake.direct != left)
				snake.direct = right;
			break;
		case 's':
		case 'S':
			if (snake.direct != up)
				snake.direct = down;
			break;
		case 'a':
		case 'A':
			if (snake.direct != right)
				snake.direct = left;
			break;
		default:
			break;
		}
	}
}

// 初始化
void init()
{
	srand((unsigned int)time(NULL));

	// 初始化游戏设置
	gp.grid_width = GRID_WIDTH;
	gp.col = COL;
	gp.row = ROW;
	gp.width = gp.col * gp.grid_width;
	gp.height = gp.row * gp.grid_width;

	cp.width = CONTROL_WIDTH;
	cp.box_left = gp.width + BORDER + PADDING;
	cp.box_right = cp.box_left + cp.width - 2 * PADDING;
	cp.box_height = BOX_HEIGHT;

	game.status = starting;			// 初始状态
	game.score = 0;					// 初始分数
	game.throutgh_the_wall = 1;		// 可以穿墙
	game.width = gp.width + BORDER + cp.width;
	game.height = gp.height;
	game.hWnd = GetHWnd();

	// 初始化设备
	initgraph(game.width, game.height);   // 创建绘图窗口
	setbkcolor(BK_COLOR);
	setbkmode(TRANSPARENT);
	cleardevice();

	LOGFONT f;
	gettextstyle(&f);                     // 获取当前字体设置
	f.lfHeight = 20;                      // 设置字体高度为 20
	_tcscpy_s(f.lfFaceName, _T("黑体"));    // 设置字体为“黑体”(高版本 VC 推荐使用 _tcscpy_s 函数)
	f.lfQuality = ANTIALIASED_QUALITY;    // 设置输出效果为抗锯齿  
	settextstyle(&f);                     // 设置字体样式
	settextcolor(BLACK);

	// 初始化蛇
	snake.direct = right;
	snake.length = 2;
	snake.body[0].left = GRID_WIDTH;		
	snake.body[0].top = 0;
	snake.body[1].left = 0;
	snake.body[1].top = 0;

	// 初始化食物
	create_food();
	//time_t t = time(NULL)
}

void create_food()
{
	food.pos.left = rand() % gp.col * gp.grid_width;
	food.pos.top = rand() % gp.row * gp.grid_width;
	time(&food.start);									// 开始时间
	food.eated = 0;
	food.score = SCORE;
}

void draw()
{
	// 绘制分隔线
	setlinecolor(BORDER_COLOR);
	setlinestyle(PS_SOLID, BORDER);
	line(gp.width + BORDER / 2, 0,
		gp.width + BORDER / 2, gp.height);

	// 绘制文字
	RECT r = { cp.box_left, 0, cp.box_right, cp.box_height };
	TCHAR text[32] = { 0 };

	// 绘制状态
	switch (game.status)
	{
	case starting:
		_stprintf_s(text, _T("状态：准备开始"));
		break;
	case running:
		_stprintf_s(text, _T("状态：游戏中"));
		break;
	case pause:
		_stprintf_s(text, _T("状态：暂停中"));
		break;
	case end:
		_stprintf_s(text, _T("状态：游戏结束"));
		break;
	}

	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	// 绘制分数
	_stprintf_s(text, _T("分数：%5d"), game.score);
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	// 绘制模式
	if (game.throutgh_the_wall) 
	{
		_stprintf_s(text, _T("模式：穿墙"));
	}
	else
	{
		_stprintf_s(text, _T("模式：普通"));
	}
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	_stprintf_s(text, _T("游戏说明"));
	r.top += cp.box_height * 2;
	r.bottom += cp.box_height * 2;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	_stprintf_s(text, _T("开始游戏：SPACE"));
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	_stprintf_s(text, _T("暂停游戏：SPACE"));
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	_stprintf_s(text, _T("结束游戏：ESC"));
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	_stprintf_s(text, _T("向左    ：A"));
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	_stprintf_s(text, _T("向右    ：D"));
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	_stprintf_s(text, _T("向上    ：W"));
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	_stprintf_s(text, _T("向下    ：S"));
	r.top = r.bottom;
	r.bottom += cp.box_height;
	drawtext(text, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	// 绘制游戏面板
	setlinestyle(PS_SOLID, 1);
	setlinecolor(LIGHTGRAY);
	for (int i = GRID_WIDTH; i < gp.height; i += gp.grid_width)
	{
		line(0, i, gp.width, i);
	}

	for (int i = GRID_WIDTH; i < gp.width; i += gp.grid_width)
	{
		line(i, 0, i, gp.height);
	}

	// 绘制食物
	if (!food.eated)
	{
		setfillcolor(FOOD_COLOR);
		fillcircle(food.pos.left + gp.grid_width / 2, food.pos.top + gp.grid_width / 2,
			gp.grid_width / 2);
	}

	// 绘制蛇
	setfillcolor(SNAKE_COLOR);
	for (int i = 0; i < snake.length; i++)
	{
		fillrectangle(snake.body[i].left, snake.body[i].top, 
			snake.body[i].left + gp.grid_width, snake.body[i].top + gp.grid_width);
	}
	
}

void move()
{
	Coordinate tail = snake.body[snake.length - 1];
	// 从倒数第一个元素到正数第二个元素前移一位
	for (int i = snake.length - 1; i >= 1; i--)
	{
		snake.body[i] = snake.body[i - 1];
	}

	// 按方向前进及穿墙
	switch (snake.direct)
	{
	case up:
		snake.body[0].top -= gp.grid_width;
		if (snake.body[0].top < 0 && game.throutgh_the_wall) 
			snake.body[0].top = gp.height - gp.grid_width;
		break;
	case right:
		snake.body[0].left += gp.grid_width;
		if (snake.body[0].left >= gp.width && game.throutgh_the_wall)
			snake.body[0].left = 0;
		break;
	case down:
		snake.body[0].top += gp.grid_width;
		if (snake.body[0].top >= gp.height && game.throutgh_the_wall)
			snake.body[0].top = 0;
		break;
	case left:
		snake.body[0].left -= gp.grid_width;
		if (snake.body[0].left < 0 && game.throutgh_the_wall)
			snake.body[0].left = gp.width - gp.grid_width;
		break;
	default:
		break;
	}

	time_t end;
	double cost;
	time(&end);
	cost = difftime(end, food.start);

	// 如果吃到食物
	if (food.eated)
	{
		snake.length += 1;
		snake.body[snake.length - 1] = tail;
		game.score += food.score;
		create_food();
	}
	// 如果大于消失时间
	else if (cost > FOOD_TIME)
	{
		create_food();
	}
}

void check()
{
	// 边界检测
	if (!game.throutgh_the_wall)
		if (snake.body[0].top < 0 || snake.body[0].top >= gp.height || snake.body[0].left < 0 || snake.body[0].left >= gp.width)
			gameover();

	// 咬到自己检测
	for (int i = 1; i < snake.length; i++)
	{
		if (snake.body[i].left == snake.body[0].left && snake.body[i].top == snake.body[0].top)
			gameover();
	}

	// 检测是否吃到食物
	if (snake.body[0].left == food.pos.left && snake.body[0].top == food.pos.top)
	{
		food.eated = 1;
	}
}

void gameover()
{
	exit(1);
}