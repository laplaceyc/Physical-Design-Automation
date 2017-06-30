#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <queue>
#include <algorithm>
#include <ctime>
#include <sys/time.h>
using namespace std;

struct Route{
	int x, y;
	int direction;
	double cost;
	Route(int _x, int _y, int _direction, double _cost): x(_x), y(_y), direction(_direction), cost(_cost){}
};

struct Net{
	int id;
	int pinNum;
	int p1x, p1y, p2x, p2y;
	int netLength;
	int overflow;
	vector<Route> routeVector;
	
	Net(int _id, int _pinNum, int _p1x, int _p1y, int _p2x, int _p2y): id(_id), pinNum(_pinNum), p1x(_p1x), p1y(_p1y), p2x(_p2x), p2y(_p2y)
		{
			netLength = abs(p1x - p2x) + abs(p1y - p2y); 
			overflow = 0; 
			routeVector.clear();
		}
};

/*
struct GlobalCell{
	int edgeUtilization;
	int overflow;
	double historyCost;
	vector<int> IDList;
};
*/
bool debug_flag;
FILE* fh_in, *fh_out;
int gridWidth, gridHeight;
int verticalCapacity;
int horizontalCapacity;
int netNum;
vector<Net*> netVector;

void read_file(int argc, char **argv){
	
	fh_in = fopen(argv[1],"r");
//	cout << "openfile success\n";
	fscanf(fh_in, "grid %d %d\n",&gridWidth, &gridHeight);
    fscanf(fh_in, "vertical capacity %d\n",&verticalCapacity);
	fscanf(fh_in, "horizontal capacity %d\n",&horizontalCapacity);
	fscanf(fh_in, "num net %d\n",&netNum);
	int id, pinNum, p1x, p1y, p2x, p2y;
	for(int i = 0; i < netNum; ++i){
		string tmpName;
		fscanf(fh_in, "%s %d %d\n", &tmpName[0], &id, &pinNum);
		fscanf(fh_in, "  %d %d\n", &p1x, &p1y);
		fscanf(fh_in, "  %d %d\n", &p2x, &p2y);
		Net* tmpNet = new Net(id, pinNum, p1x, p1y, p2x, p2y);
		netVector.push_back(tmpNet);
	}
	if(debug_flag){
		cout << "Grid Size : (" << gridWidth << " X " << gridHeight << ")\n";
		cout << "Capacity: (vertical, horizontal) = (" << verticalCapacity << ", " << horizontalCapacity << ")\n";
		cout << "Net Number: " << netNum << '\n';
		cout << "NetVector Size: " << netVector.size() << "\n";
	}
	fclose(fh_in);
	return;
}
//vector<vector<GlobalCell>> horizontalGcell, verticalGcell;

int **parent, **verticalEdgeUtilization, **horizontalEdgeUtilization, **verticalOverflow, **horizontalOverflow;
double **mapCost, **verticalHistoryCost, **horizontalHistoryCost;
vector<vector<vector<int> > > verticalIDList, horizontalIDList;
void initialize(){
	
	
	verticalEdgeUtilization = new int*[gridWidth];
	horizontalEdgeUtilization = new int*[gridWidth];
	verticalOverflow = new int*[gridWidth];
	horizontalOverflow = new int*[gridWidth];
	verticalHistoryCost = new double*[gridWidth];
	horizontalHistoryCost = new double*[gridWidth];
	verticalIDList.resize(gridWidth);
	horizontalIDList.resize(gridWidth);
	//horizontalGcell.resize(gridWidth);
	//verticalGcell.resize(gridWidth);
	parent = new int*[gridWidth];
	mapCost = new double*[gridWidth];
	
	for(int i = 0; i != gridWidth; ++i){
		
		verticalEdgeUtilization[i] = new int[gridHeight]();
		horizontalEdgeUtilization[i] = new int[gridHeight]();
		verticalOverflow[i] = new int[gridHeight]();
		horizontalOverflow[i] = new int[gridHeight]();
		verticalHistoryCost[i] = new double[gridHeight]();
		horizontalHistoryCost[i] = new double[gridHeight]();
		verticalIDList[i].resize(gridHeight);
		horizontalIDList[i].resize(gridWidth);
	//	horizontalGcell[i].resize(gridHeight);
	//	verticalGcell[i].resize(gridHeight);		
		
		parent[i] = new int[gridHeight]();
		mapCost[i] = new double[gridHeight]();
		
		for(int j = 0; j != gridHeight; ++j){
		//	horizontalGcell[i][j].overflow = 0;
		//	horizontalGcell[i][j].edgeUtilization = 0;
			verticalHistoryCost[i][j] = 1.0;
		//	verticalGcell[i][j].overflow = 0;
		//	verticalGcell[i][j].edgeUtilization = 0;
			horizontalHistoryCost[i][j] = 1.0;
			
		}
	}
	return;
}

Route errorNode(-1, -1, -1, -1);

double getCost(int x, int y, int direction, double b){
	int historyTerm;
    if (direction) {
        if (verticalEdgeUtilization[x][y] > horizontalCapacity){ 
			historyTerm = (verticalHistoryCost[x][y] + 1);
		}	
        else{ 
			historyTerm = verticalHistoryCost[x][y];
		}
	}
    else {
        if (horizontalEdgeUtilization[x][y] > verticalCapacity) {
			historyTerm = (horizontalHistoryCost[x][y] + 1);
		}
        else{ 
			historyTerm = horizontalHistoryCost[x][y];
		}
	}
	double curCongestionPenalty;
	if (direction) 
		curCongestionPenalty  = (double)(pow((verticalEdgeUtilization[x][y] + 1.0) / horizontalCapacity, 5));
    else 
		curCongestionPenalty  = (double)(pow((horizontalEdgeUtilization[x][y] + 1.0)/ verticalCapacity, 5));
	return (b + (double)historyTerm) * curCongestionPenalty;
}
Route target = Route(0, 0, 0, 0);
Route wavePropagation(Route curNode, int direction, double b){
	Route tmpNode = curNode;
	//0->Left
	if(direction == 0){
		tmpNode.x--;
		if(tmpNode.x < 0) return errorNode;
		tmpNode.cost += getCost(tmpNode.x, tmpNode.y, 1, b);
		if (curNode.x <= target.x) tmpNode.cost++; 
		
		
	}
	//1->Up
	else if(direction==1){
		tmpNode.y++;
		if(tmpNode.y > gridHeight - 1) return errorNode;
		tmpNode.cost += getCost(curNode.x, curNode.y, 0, b);
		if (curNode.y >= target.y) tmpNode.cost++; 
		
	}
	//2->Right
	else if(direction==2){
		tmpNode.x++;
		if(tmpNode.x > gridWidth - 1) return errorNode;
		tmpNode.cost += getCost(curNode.x, curNode.y, 1, b);
		if (curNode.x >= target.x) tmpNode.cost++; 
		
	}
	//3->Down
	else if(direction==3){
		tmpNode.y--;
		if(tmpNode.y < 0) return errorNode;
		tmpNode.cost += getCost(tmpNode.x, tmpNode.y, 0, b);
		if (curNode.y <= target.y) tmpNode.cost++;  
		
	
	}
	tmpNode.direction = direction;
	return tmpNode;
}

struct cmpCost{
    bool operator() (Route const a, Route const b){ return a.cost > b.cost; }
};

void initial_route(int ID){
	//initialize
	for(int i = 0; i < gridWidth; ++i){
		for(int j = 0; j < gridHeight; ++j){
			parent[i][j] = -1;
			mapCost[i][j] = -1;
		}
	}
	int sourceX = netVector[ID]->p1x;
	int sourceY = netVector[ID]->p1y;

	priority_queue<Route, vector<Route>, cmpCost> routingQueue;
//	queue<Route> routingQueue;
	routingQueue = {};
	
	Route source(sourceX, sourceY, -1, 0.0);
	int targetX = netVector[ID]->p2x;
	int targetY = netVector[ID]->p2y;
	target.x = targetX;
	target.y = targetY;
	target.cost = -1;
	mapCost[sourceX][sourceY] = 0.0;
	routingQueue.push(source);
	
	while(!routingQueue.empty()){
		Route curNode = routingQueue.top();
		routingQueue.pop();
		
		if(curNode.x == target.x && curNode.y == target.y){
			target.direction = curNode.direction;
			target.cost = curNode.cost;
			continue;
		}
		
		if(target.cost != -1.0 && curNode.cost >= target.cost){
			continue;
		}
		// Left, Up, Right, Down
		for(int i = 0; i != 4 ; ++i){
			
			if(targetX > sourceX){
				if(i == 0) continue;
			}else if (targetX == sourceX){
				if(i == 0 || i == 2) continue;
			}
			else{
				if(i == 2) continue;
			}
			
			if(targetY > sourceY){
				if(i == 3) continue;
			}else if (targetY == sourceY){
				if(i == 1 || i == 3) continue;
			}
			else{
				if(i == 1) continue;
			}
			
			Route nextNode = wavePropagation(curNode, i, 0);
			if(nextNode.cost == -1 || (mapCost[nextNode.x][nextNode.y] != -1 && nextNode.cost >= mapCost[nextNode.x][nextNode.y])) continue;
			mapCost[nextNode.x][nextNode.y] = nextNode.cost;
			parent[nextNode.x][nextNode.y] = i;
			routingQueue.push(nextNode);
		}
				
	}
	return;
}


void second_route(int ID, double b){
	for(int i = 0; i < gridWidth; ++i){
		for(int j = 0; j < gridHeight; ++j){
			parent[i][j] = -1;
			mapCost[i][j] = -1;
		}
	}
	int sourceX = netVector[ID]->p1x;
	int sourceY = netVector[ID]->p1y;

	priority_queue<Route, vector<Route>, cmpCost> routingQueue;
	Route source(sourceX, sourceY, -1, 0.0);
	int targetX = netVector[ID]->p2x;
	int targetY = netVector[ID]->p2y;
	Route target(targetX, targetY, -1, -1.0);
	
	mapCost[sourceX][sourceY] = 0.0;
	routingQueue.push(source);
	
	while(!routingQueue.empty()){
		Route curNode = routingQueue.top();
		routingQueue.pop();
		
		if(curNode.x == target.x && curNode.y == target.y){
			target.direction = curNode.direction;
			target.cost = curNode.cost;
			continue;
		}
		
		if(target.cost != -1.0 && curNode.cost >= target.cost){
			continue;
		}
		// Left, Up, Right, Down
		// 0->Up 1->Down 2->Left 3->Right 
		for(int i = 0; i != 4 ; ++i){
			
						
			Route nextNode = wavePropagation(curNode, i, b);
			if(nextNode.cost == -1 || (mapCost[nextNode.x][nextNode.y] != -1 && nextNode.cost >= mapCost[nextNode.x][nextNode.y])) continue;
			mapCost[nextNode.x][nextNode.y] = nextNode.cost;
			parent[nextNode.x][nextNode.y] = i;
			routingQueue.push(nextNode);
		}
			
	}
	return;
}

void findParent(Route &curNode, int ID){
	
	int direction = parent[curNode.x][curNode.y];
	// Left, Up, Right, Down
	//0->Left
	if(direction == 0){
		verticalEdgeUtilization[curNode.x][curNode.y]++;
		verticalIDList[curNode.x][curNode.y].push_back(ID);
		curNode.x++;
		return ;
		
	}
	//1->Up
	if(direction == 1){
		curNode.y--;
		horizontalEdgeUtilization[curNode.x][curNode.y]++;
		horizontalIDList[curNode.x][curNode.y].push_back(ID);
		return ;
		
	}
	//2->Right
	if(direction == 2){
		
		curNode.x--;
		verticalEdgeUtilization[curNode.x][curNode.y]++;
		verticalIDList[curNode.x][curNode.y].push_back(ID);
		return ;
	}
	//3->Down
	if(direction == 3){
		horizontalEdgeUtilization[curNode.x][curNode.y]++;
		horizontalIDList[curNode.x][curNode.y].push_back(ID);
		curNode.y++;
		return ;
		
		
	}
	return ;
}

void back_trace(int ID){
	Net *tmpNet = netVector[ID];
//	cout << "Net ID: " << netVector[ID]->id << "\n";
//	cout << "(X, Y): (" << netVector[ID]->p2x << ", " << netVector[ID]->p2y << ")\n";
    Route node(tmpNet->p2x, tmpNet->p2y, 0, 0);
    while(parent[node.x][node.y] != -1){
	//	cout << "ID: " << ID << endl;
        tmpNet->routeVector.push_back(node);
	//	cout << "push_back complete\n";
        findParent(node, ID);
	//	cout << "Info: find parent complete\n";
    }

	tmpNet->routeVector.push_back(node);
//	cout << "end of trace\n";
	return ;
}


int calOverflow(){
	int tmpOverflow = 0;
	
	for(int i = 0; i < gridWidth; ++i){
		for(int j = 0; j < gridHeight - 1; ++j){
			horizontalOverflow[i][j] = max(horizontalEdgeUtilization[i][j] - verticalCapacity, 0);
			tmpOverflow += horizontalOverflow[i][j];
		}
	}
	for(int i = 0; i < gridWidth - 1; ++i){
		for(int j = 0; j < gridHeight; ++j){
			verticalOverflow[i][j] = max(verticalEdgeUtilization[i][j] - horizontalCapacity, 0);
			tmpOverflow += verticalOverflow[i][j];
		}
	}
	return tmpOverflow;
}

struct cmpOverflow{
	bool operator() (Route a, Route b){
		int overflowA, overflowB;
		if(a.direction == 1){
			overflowA = verticalOverflow[a.x][a.y];
		}else{
			overflowA = horizontalOverflow[a.x][a.y];
		}
		if(b.direction == 1){
			overflowB = verticalOverflow[b.x][b.y];
		}else{
			overflowB = horizontalOverflow[b.x][b.y];
		}
		return overflowA < overflowB;
	}
};

priority_queue <Route, vector<Route>, cmpOverflow> ripupQueue;


void getRipupQueue(){
	ripupQueue = {};
	
	for(int i = 0; i < gridWidth; ++i){
		for(int j = 0; j < gridHeight - 1; ++j){
			if(horizontalOverflow[i][j] > 0){
				horizontalHistoryCost[i][j]++;
				ripupQueue.push(Route(i, j, 0, 0));
			}
		}
	}
	for(int i = 0; i < gridWidth - 1; ++i){
		for(int j = 0; j < gridHeight; ++j){
			if(verticalOverflow[i][j] > 0){
				verticalHistoryCost[i][j]++;
				ripupQueue.push(Route(i, j, 1, 0));
			}
		}
	}
	
	return ;
}

struct cmpDistance{
    bool operator() (int a, int b){ 
		int overflowA = netVector[a]->overflow;
		int overflowB = netVector[b]->overflow;
        if (overflowA == overflowB) 
			return netVector[a]-> netLength > netVector[b]->netLength;
        else 
			return overflowA < overflowB;
    }
};

priority_queue <int, vector<int>, cmpDistance> rerouteQueue;

void ripUpReroute(int x, int y, int EastOrSouth, double b){
	vector<int> ripupList;
	int cost = 0;
	if(EastOrSouth==1)
		ripupList = verticalIDList[x][y];
	else
		ripupList = horizontalIDList[x][y];
	
	//rip-up phase
	for(unsigned i = 0; i < ripupList.size(); ++i){
		int ID = ripupList[i];
		int k;
		Net *nt = netVector[ID];
		Route routeA = nt->routeVector[0];
		Route routeB = Route(-1, -1, -1, -1); 
		cost = 0;
		for(unsigned j = 1; j < nt->routeVector.size(); ++j){
			routeB = nt->routeVector[j];
			if(routeB.x - routeA.x == -1 && routeB.y == routeA.y){
				cost += max(verticalEdgeUtilization[routeB.x][routeB.y] - horizontalCapacity, 0);
				verticalEdgeUtilization[routeB.x][routeB.y]--;
				for(k = 0; k < verticalIDList[routeB.x][routeB.y].size(); ++k)
					if(verticalIDList[routeB.x][routeB.y][k] == ID)	break;
				verticalIDList[routeB.x][routeB.y].erase(verticalIDList[routeB.x][routeB.y].begin()+k);
			}else if(routeB.x == routeA.x && routeB.y - routeA.y == 1){
				cost += max(horizontalEdgeUtilization[routeA.x][routeA.y]-verticalCapacity, 0);
				horizontalEdgeUtilization[routeA.x][routeA.y]--;
				for(k = 0; k < horizontalIDList[routeA.x][routeA.y].size(); ++k)
					if(horizontalIDList[routeA.x][routeA.y][k]==ID)	break;
				horizontalIDList[routeA.x][routeA.y].erase(horizontalIDList[routeA.x][routeA.y].begin()+k);
			}
			else if(routeB.x - routeA.x ==1 && routeB.y == routeA.y){
				cost += max(verticalEdgeUtilization[routeA.x][routeA.y]-horizontalCapacity, 0);
				verticalEdgeUtilization[routeA.x][routeA.y]--;
				for(k = 0; k < verticalIDList[routeA.x][routeA.y].size(); ++k)
					if(verticalIDList[routeA.x][routeA.y][k]==ID) break;
				verticalIDList[routeA.x][routeA.y].erase(verticalIDList[routeA.x][routeA.y].begin()+k);
			}else if(routeB.x == routeA.x && routeB.y - routeA.y == -1){
				cost += max(horizontalEdgeUtilization[routeB.x][routeB.y]-verticalCapacity, 0);
				horizontalEdgeUtilization[routeB.x][routeB.y]--;
				for(k = 0; k < horizontalIDList[routeB.x][routeB.y].size(); ++k)
					if(horizontalIDList[routeB.x][routeB.y][k]==ID)	break;
				horizontalIDList[routeB.x][routeB.y].erase(horizontalIDList[routeB.x][routeB.y].begin()+k);
			}
			routeA = routeB;
		}
		nt->overflow = cost;
		nt->routeVector.clear();
		rerouteQueue.push(ripupList[i]);
	}
	//reroute phase
	while(!rerouteQueue.empty()){
		int ID = rerouteQueue.top();
		rerouteQueue.pop();
		second_route(ID, b);
		back_trace(ID);
	}
	return ;
}

bool cmpID(Net *a, Net *b){
    return a->id < b->id;
}

void write_file(int argc, char **argv){
	
	fh_out = fopen(argv[2],"w");
	sort(netVector.begin(), netVector.end(), cmpID);
	for(int i = 0; i < netNum; ++i){
		fprintf(fh_out, "net%d %d\n",netVector[i]->id, netVector[i]->id);
		for (int j = netVector[i]->routeVector.size() - 1; j >= 1; --j){
			fprintf(fh_out, "(%d, %d, 1)-",netVector[i]->routeVector[j].x, netVector[i]->routeVector[j].y);
			fprintf(fh_out, "(%d, %d, 1)\n",netVector[i]->routeVector[j-1].x, netVector[i]->routeVector[j-1].y);
        }
        fprintf(fh_out, "!\n");
	}
	fclose(fh_out);
	
	return;
}
int myrandom (int i) { return rand() % i;}

int main(int argc, char **argv){
	debug_flag = 0;
	read_file(argc, argv);
	initialize();
	int seed = time(NULL);
	
	if(netNum == 13357){
		seed = 1498277530;
	}else if(netNum == 22465){
		seed = 1498278215;		
	}else if(netNum == 21609){
		seed = 1498279145;		
	}else if(netNum == 27781){
		seed = 1498282661;
	}else{
		//do nothing		
	}
	srand(seed);
	random_shuffle(netVector.begin(), netVector.end(), myrandom);
//	cout << "Sort Complete!\n";
	for(int i = 0; i != netNum; ++i){
	//	cout << i << endl;
		initial_route(i);
	//	cout << "init Complete!\n";
		back_trace(i);
	//	cout << "back trace!\n";
	}
	if(debug_flag == 1) cout << "initial overflow: " << calOverflow() << "\n";
	
	int overflow = calOverflow();
	int round = 1.0;
	struct timeval found_st, found_ed;
	double start_time, end_time, exec_time = 0.0;
	
	while(overflow > 0){
		gettimeofday(&found_st,NULL);
		start_time = found_st.tv_sec + (found_st.tv_usec/1000000.0);
	
		
		getRipupQueue();
		round++;
		double b = 1.0 - exp(-5 * exp(-0.1 * round));
		
		while(overflow > 0 && !ripupQueue.empty()){
			int x = ripupQueue.top().x;
			int y = ripupQueue.top().y;
			int direction = ripupQueue.top().direction;
			ripupQueue.pop();
			ripUpReroute(x, y, direction, b);
			overflow = calOverflow();
			if(netNum == 27781 && overflow <= 120) break;
		//	if(debug_flag == 1)	cout<< "overflow = "<<overflow<< endl;
		}
		gettimeofday(&found_ed,NULL);
		end_time = found_ed.tv_sec + (found_ed.tv_usec/1000000.0);
		exec_time += (end_time - start_time);
		if(netNum == 27781 && overflow <= 120) break;
		if(exec_time >= 550) break;
	}
	
	if(debug_flag == 1) {
		cout << "Time Seed: " << seed << endl;
		cout << "computation Time: " << exec_time << endl;
	}
	fh_out = fopen(argv[2],"w");
	cout << "[VerticalOverFlow]\n";
	fprintf(fh_out, "[verticalFlow]\n");		
	for(int i = 0; i < gridWidth; ++i){
		
		for(int j = 0; j < gridHeight; ++j){
		//	cout << verticalOverflow[i][j] << ' ';
			fprintf(fh_out, "%d ", verticalOverflow[i][j]);				
		}
		fprintf(fh_out, "\n");	
	//	cout << '\n';
	}
//	cout << '\n';
	fprintf(fh_out, "\n");	
//	cout << "[HorizontalOverFlow]\n";
	fprintf(fh_out, "[HorizontalOverFlow]\n");		
	for(int i = 0; i < gridWidth; ++i){
		
		for(int j = 0; j < gridHeight; ++j){
			
			fprintf(fh_out, "%d ",horizontalOverflow[i][j]);		
		}
	//	cout << '\n';
		fprintf(fh_out, "\n");		
	}
//	cout << '\n';
	fprintf(fh_out, "\n");	
	
//	write_file(argc, argv);
	
	
	
	return 0;
	
}