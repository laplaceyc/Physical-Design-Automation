#include<iostream>
#include<unistd.h>
#include<cstdlib>
#include<fstream>
#include<cstdio>
#include<string>
#include<vector>
#include<list>
#include<sstream>
#include<cmath>
#include<algorithm>
#include<ctime>
#include<cstdlib>
#include<cstring>
#include<sys/time.h>

using namespace std;
FILE *fh_in_block, *fh_in_net, *fh_in_terminal;
FILE *fh_out;
double ws_ratio;//global white space ratio
int seed = (unsigned)time(NULL);


struct terminal{
	int id;
	int x_coordinate, y_coordinate;
};

struct net{
	int degree;
	list<int> block_list;
	list<int> terminal_list;
};

struct node{
	int id;
	int area;
	bool rotate;
	int width, height, x_coordinate, y_coordinate;
	int parent;
	int lchild, rchild;
	bool choose;
};



bool debug_flag;
bool time_flag;
bool seed_flag;
int seed_num = 3;
void parse_parameter(int argc, char **argv){
int ch;
	while((ch = getopt(argc, argv, "b:n:p:o:r:c:dhts")) != EOF){
		switch(ch){
		
			case 'b':
				fh_in_block = fopen(optarg, "r");
				
				if(!strcmp(optarg, "../testcase/n100.blocks")){
					
					seed_num = 0;
					
				}
				else if(!strcmp(optarg, "../testcase/n200.blocks")){
					
					seed_num = 1;
				}
				else if(!strcmp(optarg, "../testcase/n300.blocks")){
					
					seed_num = 2;
				}
				else{
				}	
				
				if(fh_in_block == NULL){
					cerr << "Error: Open block file failed...\n";
					exit(EXIT_FAILURE);
				}
				break;
				
			case 'n':
				fh_in_net = fopen(optarg, "r");
				if(fh_in_net == NULL){
					cerr << "Error: Open net file failed...\n";
					exit(EXIT_FAILURE);
				}
				break;
				
			case 'p':
				fh_in_terminal = fopen(optarg, "r");
				if(fh_in_terminal == NULL){
					cerr << "Error: Open terminal file failed...\n";
					exit(EXIT_FAILURE);
				}
				break;
				
			case 'r':
				
				if(optarg == NULL){
					cerr << "Insufficient argument! Please enter white space ratio\n";
					exit(EXIT_FAILURE);
				}
				
				ws_ratio = atof(optarg);
				break;
				
			case 'o':
				fh_out = fopen(optarg, "w");
				if(fh_out == NULL){
					cerr << "Error: Open output file failed...\n";
					exit(EXIT_FAILURE);
				}
				break;
		
			case 'd':
				debug_flag = true;
				break;
				
			
			case 't':
				time_flag = true;
				break;
			
			case 'c':
				seed = atoi(optarg);
				break;
			case 's':
				printf("Seed Assign\n");
				seed_flag = true;
				break;

			
			case 'h':
				cout << "Usage: " << argv[0] << " -b <blocks_file> -n <nets_file> -p <terminal_file> -o <output_file> -r <dead_space_ratio> \n";
				cout << "\t-d: Show detail message [Option]\n";
				cout << "\t-h: Show the command usage [Option]\n";
				cout << "\t-t: Show the time report [Option]\n";
				cout << "\t-s: Engage assignment of the best performance seed [Option]\n";
				cout << "\t-c <custom seed>: Assign a custom seed [Option]\n";
				exit(EXIT_FAILURE);
				break;
			default:
				cerr << "Wrong parameter's usage\n";
				cerr << "Usage: " << argv[0] << " -b <blocks_file> -n <nets_file> -p <terminal_file> -o <output_file> -r <dead_space_ratio> \n";
				exit(EXIT_FAILURE);
		}
	}
	return;
}
int block_num;
int terminal_num;
int total_area;

node* btree;
node* local_btree;
node* best_btree;
	
void parse_block(FILE* in){
	fscanf(in, "NumHardRectilinearBlocks : %d\n", &block_num);
	fscanf(in, "NumTerminals : %d\n\n", &terminal_num);
	btree = new node[block_num]();
	local_btree = new node[block_num]();
	best_btree = new node[block_num]();
	int count = block_num;
	int i = 0;
	while(count--){
	
	int a, b, c, d, e, f;//ignore parameter
	fscanf(in, "sb%d hardrectilinear 4 (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n", &btree[i].id, &a, &b, &c, &d, &btree[i].width, &btree[i].height, &e, &f);
	/*
	if(tmp_block->width > tmp_block->height){
		tmp_block->rotate = true;
		int tmp_value = tmp_block->width;
		tmp_block->width = tmp_block->height;
		tmp_block->height = tmp_value;
	}
	*/
	btree[i].area = (btree[i].width) * (btree[i].height);
	total_area += btree[i].area;
	++i;
	}
	if(debug_flag){
		
		printf("[FloorPlan Report]\n");
		
		printf("Total Area: %d\n", total_area);
		printf("Block Num: %d\n", block_num);
	}
	return;
}


terminal* terminal_array;
void parse_terminal(FILE* in){

	terminal_array = new terminal[terminal_num]();
	for(int i = 0; i < terminal_num; ++i){
		fscanf(in, "p%d %d %d\n", &terminal_array[i].id, &terminal_array[i].x_coordinate, &terminal_array[i].y_coordinate);
	}	
	if(debug_flag){
		printf("Terminal Num: %d\n", terminal_num);
	}
	return;
}

int net_num, pin_num;
net* net_array;

void parse_net(FILE* in){
	fscanf(in, "NumNets : %d\n", &net_num);
	fscanf(in, "NumPins : %d\n", &pin_num);
//	printf("Net Num: %d\n", net_num);
//	printf("Pin Num: %d\n", pin_num);
	int count = net_num;
	net_array = new net[net_num]();
	
	for(int i = 0; i < net_num; ++i){
	//	printf("Count num [%d]\n", count);
		
		int net_degree;
		fscanf(in, "NetDegree : %d", &net_degree);
	//	printf("Net Degree: %d\n", net_degree);
		net_array[i].degree = net_degree;
		while(net_degree--){
			int tmp_id;
			char tmp_char[10];
			fscanf(in, "%s\n", &tmp_char);
			if(tmp_char[0] == 's'){
				string tmp_string(tmp_char);
				for(char &c : tmp_string) if(!isdigit(c)) c=' ';
				stringstream ss(tmp_string);
				ss >> tmp_id;
				net_array[i].block_list.push_back(tmp_id);
			}
			else{
				string tmp_string(tmp_char);
				for(char &c : tmp_string) if(!isdigit(c)) c=' ';
				stringstream ss(tmp_string);
				ss >> tmp_id;
				net_array[i].terminal_list.push_back(tmp_id);
			}
		}
		
	}
	//printf("Net Num: %d\n", net_num);
	
	if(debug_flag){
	
		printf("Net Num: %d\n", net_num);
	}
	
	return;
}
int outline;
int contour_boundary;
void cal_outline(){
	outline = (int)sqrt(total_area * (1 + ws_ratio));
	if(debug_flag){
		printf("Outline: %d\n", outline);
	}
	return;
}
int *contour;

void init_contour(){
	contour_boundary = 3 * outline;
//	contour_boundary = 10000;
	contour = new int[contour_boundary]();
	if(debug_flag){
	/*
		for(int i = 0; i != contour_boundary; ++i) printf("%d", contour[i]);
		printf("\n");
	*/	
	}
	return;
}

void clear_contour(){

	for(int i = 0; i != contour_boundary; ++i) {
		contour[i] = 0;
	}
	
	return;
}
int update_contour(int x_coordinate, int width, int height){
	int boundary = x_coordinate + width;
	int max = 0;
	//find max y_coordinate
	for(int i = x_coordinate; i != boundary; ++i){
		if(contour[i] > max) max = contour[i];
	}
	//update y_coordinate
	int max_horizon = max + height;
	for(int i = x_coordinate; i != boundary; ++i){
		contour[i] = max_horizon;
	}
	return max;
}


int root;
int local_root;
int best_root;

void init_floorplan(){
	
	
	
	//initial B* tree
	for(int i = 0; i != block_num; ++i){
		btree[i].parent = -1;
		btree[i].lchild = -1;
		btree[i].rchild = -1;
		btree[i].choose = 0;
	}
	
	root = -1;
	int count = net_num;
	int tmp_x = 0;
	int cur_rblock;
	int last_id;
	
	for(int i = 0; i != count; ++i){
		for(list<int>::iterator j = net_array[i].block_list.begin(); j != net_array[i].block_list.end(); ++j){
			int cur = *j;
			if(btree[cur].choose == true){
				continue;
			}
                
			//place the first block
			else if(root == -1){
				root = cur;
				btree[root].choose = true;
				btree[root].x_coordinate = 0;
				btree[root].y_coordinate = update_contour(tmp_x, btree[root].width, btree[root].height);
				tmp_x += btree[root].width;
				cur_rblock = root;
				last_id = root;
			}
			else if((tmp_x) <= outline){
				//place on the same row
				btree[cur].choose = true;
				btree[last_id].lchild = cur;
				btree[cur].parent = last_id;
				last_id = cur;
				
				btree[cur].x_coordinate = tmp_x;
				btree[cur].y_coordinate = update_contour(tmp_x, btree[cur].width, btree[cur].height);
				tmp_x += btree[cur].width;
			}
			else{
				//place on the next row
				btree[cur].choose = true;
				btree[cur_rblock].rchild = cur;
				btree[cur].parent = cur_rblock;
				cur_rblock = cur;
				last_id = cur;
				
				btree[cur].x_coordinate = 0;
				btree[cur].y_coordinate = update_contour(0, btree[cur].width, btree[cur].height);
				tmp_x = btree[cur].width;
				
			}
	
	//++tmp_ptr;
		}
	}
	return;
}


int wire_length;
int local_wire_length;
int best_wire_length;
void cal_wire_length(){
	
	wire_length = 0;
	int max_x, min_x, max_y, min_y;
	for(int i = 0; i < net_num; ++i){
		max_x = 0;
		min_x = 1000000;
		max_y = 0;
		min_y = 1000000;
		for(auto m = net_array[i].block_list.begin(); m != net_array[i].block_list.end(); ++m){
			int tmp_x = btree[*m].x_coordinate + (btree[*m].width) / 2;
			int tmp_y = btree[*m].y_coordinate + (btree[*m].height) / 2;
			if (tmp_x > max_x) max_x = tmp_x;
			if (tmp_x < min_x) min_x = tmp_x;
			if (tmp_y > max_y) max_y = tmp_y;
			if (tmp_y < min_y) min_y = tmp_y;
		}	
		for(auto n = net_array[i].terminal_list.begin(); n != net_array[i].terminal_list.end(); ++n){
			int tmp_x = terminal_array[(*n)-1].x_coordinate;
			int tmp_y = terminal_array[(*n)-1].y_coordinate;
			if (tmp_x > max_x) max_x = tmp_x;
			if (tmp_x < min_x) min_x = tmp_x;
			if (tmp_y > max_y) max_y = tmp_y;
			if (tmp_y < min_y) min_y = tmp_y;
		}
		wire_length += ((max_x - min_x) + (max_y - min_y));
	}
	
	return;
}


void packing(int current, int dir){//dir 0 for update lchild, dir 1 for update rchild

	
	
	if(current == root){
		btree[current].x_coordinate = 0;
		btree[current].y_coordinate = update_contour(0, btree[current].width, btree[current].height);
	}
	else if(dir == 0){
		//update left-child
		int parent = btree[current].parent;
		btree[current].x_coordinate = btree[parent].x_coordinate + btree[parent].width;
		btree[current].y_coordinate = update_contour(btree[current].x_coordinate, btree[current].width, btree[current].height);
	}
	else{//dir == 1
		int parent = btree[current].parent;
		btree[current].x_coordinate = btree[parent].x_coordinate;
		btree[current].y_coordinate = update_contour(btree[current].x_coordinate, btree[current].width, btree[current].height);
	}
	
	if(btree[current].lchild != -1){
		packing(btree[current].lchild, 0);
	}
	if(btree[current].rchild != -1){
		packing(btree[current].rchild, 1);
	}
	return;
}

void rotate(int candidate){

	btree[candidate].rotate = !(btree[candidate].rotate);
	int tmp = btree[candidate].width;
	btree[candidate].width = btree[candidate].height;
	btree[candidate].height = tmp;
		
	
	
	return;
}

void swap(int candidate1, int candidate2){
	/*
	printf("Before swap: \n");
	printf("[Candidate1: %d], parent: %d, lchild %d, rchild %d\n", candidate1, btree[candidate1].parent, btree[candidate1].lchild, btree[candidate1].rchild);
	printf("[Candidate2: %d], parent: %d, lchild %d, rchild %d\n", candidate2, btree[candidate2].parent, btree[candidate2].lchild, btree[candidate2].rchild);
	*/
	
	//external change
	
	int candidate1_p = btree[candidate1].parent;
	int candidate2_p = btree[candidate2].parent;
	/*
	printf("Before swap: \n");
	printf("[Candidate1: %d], parent: %d, parent_lchild: %d, parent_rchild: %d\n", candidate1, candidate1_p, btree[candidate1_p].lchild, btree[candidate1_p].rchild);
	printf("[Candidate2: %d], parent: %d, parent_lchild: %d, parent_rchild: %d\n", candidate2, candidate2_p, btree[candidate2_p].lchild, btree[candidate2_p].rchild);
	*/
	if(candidate1_p == -1){
		root = candidate2;
	}
	else if(btree[candidate1_p].lchild == candidate1){
		btree[candidate1_p].lchild = candidate2;
	}
	else{
		btree[candidate1_p].rchild = candidate2;
	}
	
	if(candidate2_p == -1){
		root = candidate1;
	}
	else if(btree[candidate2_p].lchild == candidate2){
		btree[candidate2_p].lchild = candidate1;
	}
	else{
		btree[candidate2_p].rchild = candidate1;
	}
	/*
	printf("After swap: \n");
	printf("[Candidate1: %d], parent: %d, parent_lchild: %d, parent_rchild: %d\n", candidate1, candidate1_p, btree[candidate1_p].lchild, btree[candidate1_p].rchild);
	printf("[Candidate2: %d], parent: %d, parent_lchild: %d, parent_rchild: %d\n", candidate2, candidate2_p, btree[candidate2_p].lchild, btree[candidate2_p].rchild);
	*/
	/*
	printf("Before swap: \n");
	printf("[Candidate1: %d], parent: %d, parent_lchild: %d, parent_rchild: %d\n", candidate1, candidate1_p, btree[candidate1_p].lchild, btree[candidate1_p].rchild);
	printf("[Candidate2: %d], parent: %d, parent_lchild: %d, parent_rchild: %d\n", candidate2, candidate2_p, btree[candidate2_p].lchild, btree[candidate2_p].rchild);
	*/
	int candidate1_l = btree[candidate1].lchild;
	int candidate2_l = btree[candidate2].lchild;
	
	if(candidate1_l != -1) btree[candidate1_l].parent = candidate2;
	if(candidate2_l != -1) btree[candidate2_l].parent = candidate1;
	
	
	int candidate1_r = btree[candidate1].rchild;
	int candidate2_r = btree[candidate2].rchild;
	
	if(candidate1_r != -1) btree[candidate1_r].parent = candidate2;
	if(candidate2_r != -1) btree[candidate2_r].parent = candidate1;
	
	
	
	//internal change
	int tmp_parent = btree[candidate1].parent;
	btree[candidate1].parent = btree[candidate2].parent;
	btree[candidate2].parent = tmp_parent;
	
	int tmp_lchild = btree[candidate1].lchild;
	btree[candidate1].lchild = btree[candidate2].lchild;
	btree[candidate2].lchild = tmp_lchild;
	
	int tmp_rchild = btree[candidate1].rchild;
	btree[candidate1].rchild = btree[candidate2].rchild;
	btree[candidate2].rchild = tmp_rchild;
/*
	printf("After swap: \n");
	printf("[Candidate1: %d], parent: %d, lchild %d, rchild %d\n", candidate1, btree[candidate1].parent, btree[candidate1].lchild, btree[candidate1].rchild);
	printf("[Candidate2: %d], parent: %d, lchild %d, rchild %d\n", candidate2, btree[candidate2].parent, btree[candidate2].lchild, btree[candidate2].rchild);
*/
	

	int c = rand() % 4;
	if(c < 1){
	//	printf("[Case1]\n");
		rotate(candidate1);
	}
	else if(c  < 2){
	//	printf("[Case2]\n");
		rotate(candidate2);
	}
	else if(c < 3){
	//	printf("[Case3]\n");
		rotate(candidate1);
		rotate(candidate2);
	}
	else{
	//	printf("[Case4]\n");
	}
	
	return;
}

//candidate1 is the target block, candidate2 is the victim block
void move(int candidate1, int candidate2){
	
	
	if(candidate1 == candidate2) return;
	if(btree[candidate1].parent == candidate2 || btree[candidate2].parent == candidate1) return;
	
	
	swap(candidate1, candidate2);
	
	int candidate1_p = btree[candidate1].parent;
	if(btree[candidate1_p].lchild == candidate1){
		btree[candidate1_p].lchild = -1;
	}
	else{
		btree[candidate1_p].rchild = -1;
	}
	
	
	int a;
	
	
	
	//find a position for insert
	do{
		a = rand()%block_num;
	}while(a == candidate1 || (btree[a].lchild != -1 && btree[a].rchild != -1));
	
	btree[candidate1].parent = a;
	
	if(btree[a].lchild == -1){
	//	printf("Candidate: %d insert on %d rchild\n", candidate1, a);
		btree[a].lchild = candidate1;
	}
	else{
	//	printf("Candidate: %d insert on %d lchild\n", candidate1, a);
		btree[a].rchild = candidate1;
	}
	/*
	int c = rand()%2;
	if(c < 1){
		rotate(candidate1);
	}
	*/
	return;
}

void perturb(){
	int perturb_num = rand() % 2;
//	int perturb_num = 0;
	int candidate1 = rand() % block_num;
	
	if(perturb_num < 1){
	//	printf("[Rotate] C1: %d\n", candidate1);
		rotate(candidate1);
	}
	else if(perturb_num < 2){
		/*
		int candidate2;
		do{
			candidate2 = rand() % block_num;
		}while(candidate1 == candidate2 || (btree[candidate2].lchild == -1 && btree[candidate2].rchild == -1) || \
		btree[candidate1].parent == candidate2 || btree[candidate2].parent == candidate1);
		move(candidate1, candidate2);
		*/
		
		int candidate2;

		do{
			candidate2 = rand() % block_num;
		}while(candidate1 == candidate2 || btree[candidate1].parent == candidate2 || btree[candidate2].parent == candidate1);
	//	printf("[Swap] C1: %d, C2: %d\n", candidate1, candidate2);
		swap(candidate1, candidate2);
	//	swap(candidate2, candidate1);
	}
	else{
		
		int candidate2;
		candidate2 = candidate1;
		while(btree[candidate2].lchild != -1 || btree[candidate2].rchild != -1){
			int randRL = rand() % 2;
			if(randRL){
				if(btree[candidate2].lchild ==- 1)
					candidate2 = btree[candidate2].rchild;
				else
					candidate2 = btree[candidate2].lchild;
			}else{
				if(btree[candidate2].rchild == -1)
					candidate2 = btree[candidate2].lchild;
				else
					candidate2 = btree[candidate2].rchild;
			}
		}
		move(candidate1, candidate2);
	}
	return;
}
unsigned int cost;
unsigned int local_cost;
unsigned int best_cost;
bool fit_flag;
int x_boundary = 0;
int y_boundary = 0;
int local_x_boundary;
int local_y_boundary;
void cal_cost(int wire_length){

	fit_flag = false;
//	printf("[Before] Flag: %d ", fit_flag);
	x_boundary = 0;
	y_boundary = 0;
	unsigned int tmp_cost = 0;
	for(int i = 0; i < block_num; ++i){
		int tmp_x = btree[i].x_coordinate + btree[i].width;
		int tmp_y = btree[i].y_coordinate + btree[i].height;
		if(tmp_x > x_boundary) x_boundary = tmp_x;
		if(tmp_y > y_boundary) y_boundary = tmp_y;
	}
	
	for(int i = 0; i < contour_boundary; ++i){
		if(contour[i] == 0) {continue;}
		
		if(i < outline){
			if(contour[i] > outline)
				tmp_cost += ((contour[i] - outline) * 500);
		}
		else{
			tmp_cost += (contour[i] * 500);
		}
	
	
	}
	
//	printf("X-Boundary: %d, Y-Boundary: %d, Outline: %d\n", x_boundary, y_boundary, outline);
	
	if(x_boundary > outline) {
		fit_flag = true;
		tmp_cost += ((x_boundary - outline) * 50000);
	}
	if(y_boundary > outline) {
		fit_flag = true;
		tmp_cost += ((y_boundary - outline) * 50000);
	}
//	printf("[After] Flag: %d\n", fit_flag);

	tmp_cost += (wire_length * 0.1);
	
	cost = tmp_cost;

	return;
}

int best_x_boundary;
int best_y_boundary;
void store_best(){
	best_x_boundary = x_boundary;
	best_y_boundary = y_boundary;
	best_root = root;
	best_wire_length = wire_length;
	best_cost = cost;
	for(int i = 0; i < block_num; ++i){
		best_btree[i] = btree[i];
	
	}

	return;
}

void store_local(){
	local_x_boundary = x_boundary;
	local_y_boundary = y_boundary;
	local_root = root;
	local_wire_length = wire_length;
	local_cost = cost;
	for(int i = 0; i < block_num; ++i){
		local_btree[i] = btree[i];
		

	}

	return;
}

void restore_local(){
	
	x_boundary = local_x_boundary;
	y_boundary = local_y_boundary;
	root = local_root;
	wire_length = local_wire_length;
	cost = local_cost;
	for(int i = 0; i < block_num; ++i){
		btree[i] = local_btree[i];
		

	}
	
}

void restore_best(){
	
	x_boundary = best_x_boundary;
	y_boundary = best_y_boundary;
	root = best_root;
	wire_length = best_wire_length;
	cost = best_cost;
	for(int i = 0; i < block_num; ++i){
		btree[i] = best_btree[i];
	}

	return;
}

int found = 0;

void simulated_annealing(){
	
	
	
	
	clear_contour();
	packing(root, 0);
	cal_wire_length();
	int get_worse = 0;
	cal_cost(wire_length);
	if(debug_flag){
		printf("Initial Cost: %d\n", cost);
	}
	struct timeval found_st, found_ed, first;
	double first_time, end_time;
	while(1){
		
		store_local();
		
		
		
		perturb();

		clear_contour();
		packing(root, 0);
		cal_wire_length();
		cal_cost(wire_length);
		
		if(cost < local_cost && found == 0) {
		
			store_best();
			get_worse = 0;
			
			if(fit_flag == false) {
			
				gettimeofday(&first,NULL);
				first_time = first.tv_sec + (first.tv_usec/1000000.0);
				
				found = 1;
		
				continue;
			}
			else continue;
		}
		
		if(cost < best_cost && found == 1) {
		
			store_best();
			get_worse = 0;
			
			
			
			if(fit_flag == false) {
				gettimeofday(&found_st,NULL);
				first_time = found_st.tv_sec + (found_st.tv_usec/1000000.0);
				
	
				continue;
			}
			else continue;
		}
		
		++get_worse;
		
		if(get_worse < 20000){
			
			restore_local();
		}
		
		if(get_worse == 20000){
		
			get_worse = 0;
			
			for(int i = 0; i < 10; ++i){
				perturb();
			}
				clear_contour();
				packing(root, 0);
				cal_wire_length();
				cal_cost(wire_length);
				
		}
		
		gettimeofday(&found_ed,NULL);
		end_time = found_ed.tv_sec + (found_ed.tv_usec/1000000.0);
		if(end_time - first_time >= 5 && found == 1){
			break;
		}
			
	}//end of while loop
	
	return;
	}
		
		
		

		
	
	
	

void write_solution(FILE* out){

	
	fprintf(out, "Wirelength %d\n", wire_length);
	fprintf(out, "Blocks\n");
	
	for(int i = 0; i < block_num; ++i){
		fprintf(out, "sb%d %d %d %d\n", i, btree[i].x_coordinate, btree[i].y_coordinate, btree[i].rotate);
	}

	return;
}



int main(int argc, char **argv){
	
	struct timeval in_st, in_ed, out_st, out_ed, co_st, co_ed;
	double io_time = 0.0, comp_time = 0.0;
	double io_st1, io_ed1, io_st2, io_ed2, co_st1, co_ed1;

	
		
	parse_parameter(argc, argv);
	
	if(time_flag == true){
		gettimeofday(&in_st,NULL);
		io_st1 = in_st.tv_sec + (in_st.tv_usec/1000000.0);
	}
		
	parse_block(fh_in_block);
	fclose(fh_in_block);

	parse_terminal(fh_in_terminal);
	fclose(fh_in_terminal);
	parse_net(fh_in_net);
	fclose(fh_in_net);
	
	if(time_flag == true){
		gettimeofday(&in_ed,NULL);
		io_ed1 = in_ed.tv_sec + (in_ed.tv_usec/1000000.0);
		io_time += (io_ed1 - io_st1);
	}
	
	if(time_flag == true){
		gettimeofday(&co_st,NULL);
		co_st1 = co_st.tv_sec + (co_st.tv_usec/1000000.0);
	}
	
	
	
	if(seed_flag){
		
		if(seed_num == 0){
			if(ws_ratio == 0.15) seed = 1494164774;
			else if(ws_ratio == 0.1) seed = 1494165111;
		}else if(seed_num == 1){
			if(ws_ratio == 0.15) seed = 1494156038;
			else if(ws_ratio == 0.1) seed = 1494156580;
		}else if(seed_num == 2){
			if(ws_ratio == 0.15) seed = 1494158465;
			else if(ws_ratio == 0.1) seed = 1494157478;
		}
		
	}
	
	srand(seed);
	
	
	cal_outline();
	init_contour();
	init_floorplan();


	
	struct timeval st, ed;
	double st1, ed1;
	gettimeofday(&st,NULL);
	st1 = st.tv_sec + (st.tv_usec/1000000.0);
	
	simulated_annealing();
		
	gettimeofday(&ed,NULL);
	ed1 = ed.tv_sec + (ed.tv_usec/1000000.0);
	
	restore_best();
	clear_contour();
	packing(root, 0);
	if(debug_flag){
		printf("X-Boundary: %d, Y-Boundary: %d, Outline: %d\n", best_x_boundary, best_y_boundary, outline);
		printf("SA Time: %lf\n", ed1 - st1);
		printf("Wire Length: %d\n", wire_length);
		printf("End Seed %d\n", seed);
	}
	
	cal_wire_length();
	
	if(time_flag == true){
		gettimeofday(&co_ed,NULL);
		co_ed1 = co_ed.tv_sec + (co_ed.tv_usec/1000000.0);
		comp_time += (co_ed1 - co_st1);
	}
	
	if(time_flag == true){
		gettimeofday(&out_st,NULL);
		io_st2 = out_st.tv_sec + (out_st.tv_usec/1000000.0);
	}
	cal_wire_length();
	write_solution(fh_out);
	fclose(fh_out);
	if(time_flag == true){
		gettimeofday(&out_ed,NULL);
		io_ed2 = out_ed.tv_sec + (out_ed.tv_usec/1000000.0);
		io_time += (io_ed2 - io_st2);
	}
	
	if(time_flag == true){
		printf("[Time Report]\n");
		printf("I/O time: %lf\n", io_time);
		printf("Computing time: %lf\n", comp_time);
		printf("Total execution time: %lf\n", io_time + comp_time);
	}
	
	
	return 0;
}