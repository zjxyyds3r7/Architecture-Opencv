#define _CRT_SECURE_NO_DEPRECATE

#pragma comment(linker, "/STACK:102400000,102400000") //增加栈
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <queue>
#include <map>
#include <opencv2/opencv.hpp>
// #include <sys/wait.h>
// #include <sys/types.h>
// #include <unistd.h>
using namespace std;
#define MAXN 10000
int vis[MAXN];
cv::Scalar cols[10] = { cv::Scalar(255,0,0),cv::Scalar(100,100,100),cv::Scalar(0,255,0),cv::Scalar(0,0,255) };
cv::Mat img(1000, 1000, CV_8UC3, cv::Scalar(255, 255, 255));
int minx = 1e9, miny = 1e9, maxx = -1e9, maxy = -1e9;
struct Point //点的定义
{
	double x, y;
	Point(double x = 0, double y = 0) :x(x), y(y) {}
	bool operator ==(const Point& a)const
	{
		return x == a.x && y == a.y;
	}
	Point operator - (const Point& a)const
	{
		return Point(x - a.x, y - a.y);
	}
	Point operator + (const Point& a)const
	{
		return Point(x + a.x, y + a.y);
	}
};
double Cross(const Point& a, const Point& b) { return a.x * b.y - b.x * a.y; }
deque <Point> node, door, door_ori;

int f[MAXN];
struct Graph//图的定义
{
	int head[MAXN] = { 0 }, _edge = 1;
	Graph() {};
	struct EDGE
	{
		int a, b, type, id, next;
		//type:原边0，派生边1
		double area()const
		{
			Point x = node[b] - node[a];
			if (x.y > 0 && x.x > 0)return 1;
			if (x.y > 0 && x.x == 0)return 1.5;
			if (x.y > 0 && x.x < 0)return 2;
			if (x.y == 0 && x.x < 0)return 2.5;
			if (x.y < 0 && x.x < 0)return 3;
			if (x.y < 0 && x.x == 0)return 3.5;
			if (x.y < 0 && x.x>0)return 4;
			return 0.5;
			// if (x.y==0&&x.x>0)return 0.5;
		}
		double k()const
		{
			Point x = node[b] - node[a];
			return double(x.y) / x.x;
		}
		EDGE(int a = 0, int b = 0, int id = 0, int type = 0, int next = 0) :a(a), b(b), id(id), type(type), next(next) {}
	}edge[MAXN];
	void adde(int a, int b, int type)
	{
		++_edge;//
		/*0 1 10 11 100 101 110 111
		  0 1 2  3  4   5   6   7
		  0 0 1  1  2   2
		*/
		edge[_edge] = EDGE(a, b, _edge >> 1, type, head[a]);
		head[a] = _edge;
	}
}graph;

struct Polygon//多边形类
{
	deque<Point>q;
	void reverse()
	{
		for (int i = 0; i < q.size() >> 1; ++i)
		{
			swap(q[i], q[q.size() - i - 1]);
		}
	}
	double getS()
	{
		double ans = 0;
		for (int i = 1; i < q.size(); ++i)
		{
			ans += Cross(q[i - 1], q[i]);
		}
		return ans / 2;
	}
	void dfs(const Graph& g, int a, int f)
	{
		q.push_back(node[a]);
		if (f != -1 && q.back() == q.front())return;
		for (int i = g.head[a]; i; i = g.edge[i].next)
		{
			if (g.edge[i].b == f)continue;
			dfs(g, g.edge[i].b, a);
		}
	}
	void format(const Graph& x)
	{
		q.clear();
		dfs(x, x.edge[2].a, -1);
		if (getS() < 0)reverse();

	}
};
/* Variable explain:

*/
void dfs(int a, int v)//遍历求连通块
{
	if (vis[a])return;
	vis[a] = v;
	for (int i = graph.head[a]; i; i = graph.edge[i].next)
	{
		dfs(graph.edge[i].b, v);
	}
}

int find(int x) { return x == f[x] ? x : f[x] = find(f[x]); }//查找

//
/*
0 1 2 3 4 5
0 1 2 5 4 3
*/
void merge(int a, int b) { f[find(b)] = find(a); }//合并
//(ring[i - 1].id << 1) ^ ring[i - 1].type, (ring[i].id << 1) ^ 1 ^ ring[i].type
void read(int argc, char* argv[])//读取数据
{
	string file[3];
	// 这里改文件路径！
	file[0] = "D:\\hpvc6\\hpCV6\\x64\\Debug\\点.txt";
	file[1] = "D:\\hpvc6\\hpCV6\\x64\\Debug\\边.txt";
	file[2] = "D:\\hpvc6\\hpCV6\\x64\\Debug\\门点坐标.txt";
	FILE* in = fopen(file[0].c_str(), "r");
	int ls1, ls2;
	while (fscanf(in, "%d%d", &ls1, &ls2) != -1)
	{
		node.push_back(Point(ls1, ls2));
		minx = min(minx, ls1);
		maxx = max(maxx, ls1);
		miny = min(miny, ls2);
		maxy = max(maxy, ls2);
		//cout << "E";
	}
	in = fopen(file[2].c_str(), "r");
	while (fscanf(in, "%d%d", &ls1, &ls2) != -1)
	{
		door.push_back(Point(ls1, ls2));
	}
	door_ori = deque<Point>{ door };

	in = fopen(file[1].c_str(), "r");
	while (fscanf(in, "%d%d", &ls1, &ls2) != -1)
	{
		graph.adde(ls1, ls2, 0);
		graph.adde(ls2, ls1, 1);
	}
	double rate = 600.0 / max(maxx - minx, maxy - miny);
	//maxx -= minx, maxy -= miny;
	for (int i = 0; i < node.size(); ++i)node[i].x = (node[i].x - minx) * rate + 50, node[i].y = (maxy - miny) * rate - (node[i].y - miny) * rate + 200;
	for (int i = 0; i < door.size(); ++i)door[i].x = (door[i].x - minx) * rate + 50, door[i].y = (maxy - miny) * rate - (door[i].y - miny) * rate + 200;
	for (int i = 2; i <= graph._edge; i += 2)
	{
		cv::line(img, cv::Point(node[graph.edge[i].a].x, node[graph.edge[i].a].y), cv::Point(node[graph.edge[i].b].x, node[graph.edge[i].b].y), cv::Scalar(0, 0, 0));
	}
	for (int i = 0; i < door.size(); i++)
	{
		cv::circle(img, cv::Point(door[i].x, door[i].y), 3, cv::Scalar(0, 0, 0));
	}
	return;
}
void init()//预处理
{
	for (int i = 0; i <= graph._edge; ++i) f[i] = i;
}
void solve()//主算法
{
	for (int a = 0; a < node.size(); ++a)//遍历节点
	{
		deque<Graph::EDGE>ring;
		for (int i = graph.head[a]; i; i = graph.edge[i].next)
		{
			ring.push_back(graph.edge[i]);
		}
		sort(ring.begin(), ring.end(), [](const Graph::EDGE& a, const Graph::EDGE& b) {
			if (a.area() != b.area())return a.area() < b.area();
			return a.k() < b.k();
			});
		if (ring.empty())continue;
		ring.push_back(ring[0]);//生成环
		for (int i = 1; i < ring.size(); ++i)
		{
			printf("合并%d %d\n",(ring[i-1].id<<1)^ring[i-1].type,(ring[i].id<<1)^1^ring[i].type);
			merge((ring[i - 1].id << 1) ^ ring[i - 1].type, (ring[i].id << 1)  ^ ring[i].type ^ 1);
			//11	10
			//0<<1-> 0^1 ->1
			// cv::line(img,cv::Point(node[ring[i].a].x,node[ring[i].a].y),cv::Point(node[ring[i].b].x,node[ring[i].b].y),cols[i]);
		}
	}
}

int main(int argc, char* argv[])
{
	// freopen(".in","r",stdin);
	// freopen(".out","w",stdout);
	printf("开始\n");
	read(argc, argv);
	printf("初始化\n");
	init();
	int _vis = 0;
	for (int i = 0; i < node.size(); ++i)
	{
		if (!vis[i]) dfs(i, ++_vis);
	}
	solve();
	map<int, deque<int> >dic, edge2room, edge2door, room2edge;
	map<int, int> roomk2i, door2edge, i2roomk;
	for (int i = 2; i <= graph._edge; ++i)
	{
		dic[find(i)].push_back(i);
	}
	// 门边对应关系
	for (int i = 0; i < door.size(); i++) {
		for (int j = 2; j <= graph._edge; j = j + 2)
		{
			int e_minx = min(node[graph.edge[j].a].x, node[graph.edge[j].b].x);
			int e_maxx = max(node[graph.edge[j].a].x, node[graph.edge[j].b].x);
			int e_miny = min(node[graph.edge[j].a].y, node[graph.edge[j].b].y);
			int e_maxy = max(node[graph.edge[j].a].y, node[graph.edge[j].b].y);

			if (e_minx <= (int)door[i].x && (int)door[i].x <= e_maxx && e_miny <= (int)door[i].y && (int)door[i].y <= e_maxy) {
				//printf("%d,%d,%d,%d %d,%d\n", e_minx, e_maxx, e_miny, e_maxy, (int)door[i].x, (int)door[i].y);
				door2edge[i] = graph.edge[j].id;
				edge2door[graph.edge[j].id].push_back(i);
				break;
			}
		}
	}
	// 删除重复房间
	deque<int> del_idx;
	auto it_next = dic.begin();
	it_next++;
	for (auto it = dic.begin(); it != dic.end(); ++it) {
		if (it_next != dic.end()) {
			if (it->second.size() == it_next->second.size()) {
				bool eq_flag = 1;
				for (int i = 0; i < it->second.size(); ++i) {
					//cout << "(" << it->second[i] << "," << it_next->second[i] << ")";
					if (abs(it->second[i] - it_next->second[i]) != 1) {
						eq_flag = 0;
						break;
					}
				}
				//cout << endl;
				if (eq_flag) {
					del_idx.push_back(it->first);
				}
			}
			it_next++;
		}
	}
	for (int i = 0; i < del_idx.size(); ++i) {
		dic.erase(del_idx[i]);
	}

	int cnt = 0;
	for (auto it = dic.begin(); it != dic.end(); ++it)
	{
		printf("第%d个多边形：", ++cnt);
		roomk2i[it->first] = cnt;
		i2roomk[cnt] = it->first;
		int flag = 0;
		if (vis[graph.edge[it->second[0]].a] != 1)
		{
			flag = 1;
		}
		for (int i = 0; i < it->second.size(); ++i)
		{
			int id = it->second[i];
			cv::line(img, cv::Point(node[graph.edge[id].a].x, node[graph.edge[id].a].y), cv::Point(node[graph.edge[id].b].x, node[graph.edge[id].b].y), cols[3]);
			printf("%d ", graph.edge[id].id);
		}
		puts("");
		cv::imshow("", img);
		cv::waitKey();
	}
	// 得到边对房间关系
	for (int i = 2; i <= graph._edge; ++i)
	{
		int room_id = roomk2i[find(i)];
		edge2room[graph.edge[i].id].push_back(room_id);
		room2edge[room_id].push_back(graph.edge[i].id);
	}

	printf("孤岛\n");
	static Polygon island[100];
	static Graph ansg[100];
	double S[100] = { 0 };
	for (auto it = dic.begin(); it != dic.end(); ++it)//找到孤立多边形
	{
		int lid = vis[graph.edge[it->second[0]].a];
		if (lid != 1)
		{
			Graph ls;
			Polygon now;
			double lss = 0;
			for (int i = 0; i < it->second.size(); ++i)
			{
				int id = it->second[i];
				ls.adde(graph.edge[id].a, graph.edge[id].b, 0);
				ls.edge[ls._edge].id = graph.edge[id].id;
				// printf("(%d,%d)\n",graph.edge[id].a,graph.edge[id].b);
			}
			now.format(ls);
			if (now.getS() > S[lid]) S[lid] = now.getS(), island[lid] = now, ansg[lid] = ls;
		}
	}
	for (int T = 1; T <= _vis; ++T)
	{
		//cout << T << endl;
		if (ansg[T]._edge == 0)continue;
		for (int i = 2; i <= ansg[T]._edge; ++i)
		{
			printf("%d ", ansg[T].edge[i].id);
		}
		puts("");
		for (int i = 1; i < island[T].q.size(); ++i)//输出孤立多边形
		{
			// printf("(%.0lf,%0.lf)\n",island.q[i].x,island.q[i].y);
			// printf("(%.0lf,%0.lf)\n",island.q[i].x,island.q[i].y);
			cv::line(img, cv::Point(island[T].q[i - 1].x, island[T].q[i - 1].y), cv::Point(island[T].q[i].x, island[T].q[i].y), cols[0]);
		}
		cv::imshow("", img);
		cv::waitKey();
	}
	printf("门id  坐标  墙线  房间ID\n");
	for (int i = 0; i < door.size(); i++) {
		printf("%d    (%d, %d)    %d    ( ", i + 1, (int)door_ori[i].x, (int)door_ori[i].y, door2edge[i]);
		for (int j = 0; j < edge2room[door2edge[i]].size(); j++) {
			printf("%d ", edge2room[door2edge[i]][j]);
		}
		printf(")\n");
	}

	printf("房间id  墙线id  房间中心坐标   门数量\n");
	for (int i = 1; i <= cnt; i++) {
		int door_count = 0;
		printf("%d    ( ", i);
		for (int j = 0; j < room2edge[i].size(); j++) {
			printf("%d ", room2edge[i][j]);
			door_count = door_count + edge2door[room2edge[i][j]].size();
		}
		double tmpx = 0;
		double tmpy = 0;
		for (int j = 0; j < dic[i2roomk[i]].size(); j++) {
			tmpx = tmpx + node[graph.edge[dic[i2roomk[i]][j]].a].x;
			tmpy = tmpy + node[graph.edge[dic[i2roomk[i]][j]].a].y;
		}
		tmpx = tmpx / dic[i2roomk[i]].size();
		tmpy = tmpy / dic[i2roomk[i]].size();
		printf(")  ( %d, %d )  %d\n", (int)tmpx, (int)tmpy, door_count);

	}
	puts("");
	return 0;
}
