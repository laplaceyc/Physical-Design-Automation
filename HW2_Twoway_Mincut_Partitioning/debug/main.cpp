#include<iostream>
#include<unistd.h>
#include<cstdlib>
#include<fstream>
#include<list>
#include<vector>
#include<string>
#include<map>
#include<algorithm>
#include<stack>
#include<cmath>
using namespace std;

struct cell{
	string name;//cell name
	int size;//cell size
	int gain;
	int pin;
	vector<int> net_list;
	bool state;//0 for free; 1 for lock;
	bool partition;//0 for partition A; 1 for partition B
	list<cell*>::iterator ptr;//point to position in bucket list
};

struct net{
	string name;//net name
	int A, B;//cell num in A and B respectively
	vector<int> cell_list;
	
};

ifstream fh_in_cell, fh_in_net;
ofstream fh_out;

map<string, int> cell_dictionary, net_dictionary;
int cell_num, net_num;

vector<cell*> cell_vector;
vector<net*> net_vector;
bool debug_flag;



void parse_parameter(int argc, char **argv){
	
	int ch;
	while((ch = getopt(argc, argv, "c:n:o:dh")) != EOF){
		switch(ch){
			case 'c':
				fh_in_cell.open(optarg);
				if(!fh_in_cell.is_open()){
					cerr << "Error: Open cell file failed...\n";
					exit(EXIT_FAILURE);
				}
				break;
			case 'n':
				fh_in_net.open(optarg);
				if(!fh_in_net.is_open()){
					cerr << "Error: Open net file failed...\n";
					exit(EXIT_FAILURE);
				}
				break;
        
			case 'o':
				fh_out.open(optarg);
				if(!fh_out.is_open()){
					cerr << "Error: Open output file failed...\n";
					exit(EXIT_FAILURE);
				}
				break;

			case 'd':
				debug_flag = true;
				break;
			case 'h':
				cout << "Usage: " << argv[0] << " [-c cells_file] [-n nets_file] [-o output_file]\n";
				cout << "\t-d: Print debug message\n";
				break;
			default:
				cerr << "Wrong parameter's usage\n";
				cerr << "Usage: " << argv[0] << "[-c cells_file] [-n nets_file] [-o output_file]\n";
				exit(EXIT_FAILURE);
		}
	}
}

void parse_cell(istream &in){
	string tmp_name;
	int tmp_size;
	while(in >> tmp_name >> tmp_size){
		cell_dictionary[tmp_name] = cell_num;
		cell* tmp_cell = new cell();
		tmp_cell->name = tmp_name;
		tmp_cell->size = tmp_size;
		cell_vector.push_back(tmp_cell);
		++cell_num;
	}
	if(debug_flag == true){
		cout << "Summary: \n";
		cout << "total cell num =" << cell_num << '\n';
	}
	return;
}

void parse_net(istream &in){
	string title;
	while(in >> title){
		string tmp_name;
		in >> tmp_name;
		net_dictionary[tmp_name] = net_num;
		net* tmp_net = new net();
		tmp_net->name = tmp_name;
		net_vector.push_back(tmp_net);
		string cell_name;
		while(in >> cell_name){
			if(cell_name[0] == '{') { continue; }
			if(cell_name[0] == '}') { break; }
			cell* &tmp_cell = cell_vector[cell_dictionary[cell_name]];
			tmp_cell->net_list.push_back(net_num);
			++(tmp_cell->pin);
			net_vector[net_num]->cell_list.push_back(cell_dictionary[cell_name]);
		}
		++net_num;
	}
	if(debug_flag == true){
/*		
		for(vector<net*>::iterator i = net_vector.begin(); i != net_vector.end(); i++){
//			cout << (*i)->name << " " << (*i)->A << ' ' << (*i)->B << '\n';
			for(vector<int>::iterator j = (*i)->cell_list.begin(); j != (*i)->cell_list.end(); j++){
				cout << *j << ' ';
			}
			cout << '\n';
		}
*/		
/*		
		for(vector<cell*>::iterator i = cell_vector.begin(); i != cell_vector.end(); i++){
			cout << (*i)->name << " " << (*i)->size << ' ' << (*i)->pin << ' ' <<(*i)->state << '\n';
			if((*i)->state == 0) partA_size += (*i)->size;
			else partB_size += (*i)->size;
		}
*/		
		cout << "Summary: \n";
		cout << "total net num =" << net_num << '\n';
		
	}
	return;
}

bool cmppin(cell* &x, cell* &y){
	return x->pin < y->pin;
}

int partA_num, partB_num;
int partA_size, partB_size;
void initial_partition(){
	int n = cell_num;
	list<cell*> heap;
	for(int i = 0; i < n; ++i){
		heap.push_back(cell_vector[i]);
	}
//	heap.sort(cmppin);
/*
	cout << "List info. begin \n";
	for(list<cell*>::iterator i = heap.begin(); i != heap.end(); ++i){
		cout << (*i)->name << ' ' << (*i)->size << ' ' << (*i)->pin << '\n';
	}
	cout << "List info. end \n";
*/	
	while(!heap.empty()){
		if(partA_size <= partB_size){
			cell* &tmp = heap.back();
			tmp -> partition = 0;
			partA_size += tmp->size;
			++partA_num;
			heap.pop_back();
		}
		else{
			cell* &tmp = heap.front();
			tmp -> partition = 1;
			partB_size += tmp->size;
			++partB_num;
			heap.pop_front();
		}
	}
	if(debug_flag == true){
		cout << "Partition A num = " << partA_num << " size = "<< partA_size << '\n';
		cout << "Partition B num = " << partB_num << " size = "<< partB_size << '\n';
	}
	return;
}

void get_AB(){
    for (int i = 0; i < net_num; ++i){
        vector<int> &tmp_vector = net_vector[i]->cell_list;
        net_vector[i]->B = 0;
        net_vector[i]->A = 0;
        for (int j = 0; j < tmp_vector.size(); ++j){
            cell* tmp_cell = cell_vector[tmp_vector[j]];
            if (tmp_cell->partition) net_vector[i]->B++;
            else net_vector[i]->A++;
        }
    }
}

int cutsize;
void get_cutsize(){
	cutsize = 0;
	for(int i = 0; i < net_num; i++){
		if(net_vector[i]->A && net_vector[i]->B) cutsize++;
	}
	if(debug_flag == true){
		cout << "cut size: " << cutsize << '\n';
	}
	return;
}

int constraint;
void get_constraint(){
	constraint = 0;
	constraint = cell_num / 10;
	if(debug_flag == true){
		cout << "constraint: " << constraint << '\n';
	}
}

int maxpin;
void get_pmax(){
	maxpin = 0;
	for(int i = 0; i < cell_num; ++i){
		int pin_num = cell_vector[i]->pin;
		if(maxpin < pin_num) maxpin = pin_num;
	}
	if(debug_flag == true){
		cout << "Max pin num: " << maxpin << '\n';
	}
	return;
}


void initial_gain(){
	for(int i = 0; i < cell_num; ++i){
		cell_vector[i]->gain = 0;
		cell_vector[i]->state = false;
	}
	for(int i = 0; i < cell_num; ++i){
		for(int j = 0; j < cell_vector[i]->net_list.size(); ++j){
			int id = cell_vector[i]->net_list[j];
			if(cell_vector[i]->partition == 0){
				if(net_vector[id]->A == 1){
					cell_vector[i]->gain++;
				}
				if(net_vector[id]->B == 0){
					cell_vector[i]->gain--;
				}
			}
			else{
				if(net_vector[id]->B == 1){
					cell_vector[i]->gain++;
				}
				if(net_vector[id]->A == 0){
					cell_vector[i]->gain--;
				}
			}
				
		}
	}
	return;
}

map<int, list<cell*> > bucketA, bucketB;
void get_bucket_list(){
	bucketA.clear();
	bucketB.clear();
	for(int i = -maxpin; i <= maxpin; ++i){
		list<cell*> gain_listA, gain_listB;
		bucketA[i] = gain_listA;
		bucketB[i] = gain_listB;
	}
	for(int i = 0; i < cell_num; ++i){
		int g = cell_vector[i]->gain;
		bool s = cell_vector[i]->partition;
		cell *position = new cell();
		position->name = cell_vector[i]->name;
		if(s == 0) {
			bucketA[g].push_back(position);
			cell_vector[i]->ptr = --(bucketA[g].end());
		}
		else {
			bucketB[g].push_back(position);
			cell_vector[i]->ptr = --(bucketB[g].end());
		}
	}
/*	
	if(debug_flag == true){
		cout << "------------------------------------\n";
		cout << "Map1\n";
		for(int i = maxpin; i >= -maxpin; --i){
			cout << "[ " << i << " ] ";
			for(list<cell*>::iterator j = bucketA[i].begin(); j != bucketA[i].end(); ++j){
			cout << (*j)->name << ' ';
			}
			cout << '\n';
		}
		cout << "Map2\n";
		for(int i = maxpin; i >= -maxpin; --i){
			cout << "[ " << i << " ] ";
			for(list<cell*>::iterator j = bucketB[i].begin(); j != bucketB[i].end(); ++j){
				cout << (*j)->name << ' ';
			}
			cout << '\n';
		}
		cout << "------------------------------------\n";
	}
*/	
	return;
}

string get_maxgain_cellname(bool partition){
	string cell_name("");
	if(partition == 0){
		for(int i = maxpin; i >= -maxpin; --i){
			if(bucketA[i].empty()) { continue; }
			for(list<cell*>::iterator j = bucketA[i].begin(); j != bucketA[i].end(); ++j){
//				if((*j)->state == true) { continue; }
				cell_name = (*j)->name;
//				(*j)->state = true;
//				bucketA[i].erase(j);
				break;
			}
			if(cell_name == "") { continue; }
			break;
		}
	}
	else {
		for(int i = maxpin; i >= -maxpin; --i){
			if(bucketB[i].empty()) { continue; }
			for(list<cell*>::iterator j = bucketB[i].begin(); j != bucketB[i].end(); ++j){
//				if((*j)->state == true) { continue; }
				cell_name = (*j)->name;
//				(*j)->state = true;
//				bucketB[i].erase(j);
				break;
			}
			if(cell_name == "") { continue; }
			break;
		}
	}
	return cell_name;
}

void remove_cell(string tmp){
	if(cell_vector[cell_dictionary[tmp]]->partition == 1){
		bucketA[cell_vector[cell_dictionary[tmp]]->gain].erase(cell_vector[cell_dictionary[tmp]]->ptr);
	}
	else {
		bucketB[cell_vector[cell_dictionary[tmp]]->gain].erase(cell_vector[cell_dictionary[tmp]]->ptr);
	}
	return;
}


void update_gain(string tmp){

	cell* &tmp_cell = cell_vector[cell_dictionary[tmp]];
	tmp_cell->state = true;
	if(!(tmp_cell->partition)){//from A to B
//	cout << "In condition B\n";
		int n = tmp_cell->net_list.size();
//		cout << "net size: " << n << '\n';
		for(int i = 0; i != n; ++i){
			net* &tmp_net = net_vector[tmp_cell->net_list[i]];
			int m = tmp_net->cell_list.size();
	//		cout << "net name " << tmp_net->name <<" size " << m<<'\n';
			int origin_gain[m];
	//		cout << "og list:\n";
			for(int j = 0; j != m; ++j){
				origin_gain[j] = cell_vector[tmp_net->cell_list[j]]->gain;
		//		cout << cell_vector[tmp_cell->net_list[j]]->name << ' ' << origin_gain[j] << '\n';
			}
//			cout << '\n';
			//before the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->B == 0){
					if(cell_for_update->state == false)	cell_for_update->gain++;
				}
				else if(tmp_net->B == 1){
					if((cell_for_update->partition == 1) && (cell_for_update->state == false)) cell_for_update->gain--;
				}
			}
			//F(n) = F(n) - 1; T(n) = T(n) + 1;
			tmp_net->A--;
			tmp_net->B++;
			tmp_cell->partition = true;
			//after the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->A == 0){
					if(cell_for_update->state == false)	cell_for_update->gain--;
				}
				else if(tmp_net->A == 1){
					if((cell_for_update->partition == 0) && (cell_for_update->state == false)) cell_for_update->gain++;
				}
			}
			
			//adjust location on bucketlist A
			for(int j = 0; j != m; ++j){
				if(cell_vector[tmp_net->cell_list[j]]->state == true){ continue; }
//				cout << origin_gain[j] << ' ';
				if(cell_vector[tmp_net->cell_list[j]]->partition == false){//in bucket list A
					bucketA[origin_gain[j]].erase(cell_vector[tmp_net->cell_list[j]]->ptr);
					cell* insert_cell = new cell();
					insert_cell->name = cell_vector[tmp_net->cell_list[j]]->name;
					bucketA[(cell_vector[tmp_net->cell_list[j]])->gain].push_front(insert_cell);
					cell_vector[tmp_net->cell_list[j]]->ptr = bucketA[cell_vector[tmp_net->cell_list[j]]->gain].begin();
				}
				else{//in bucket list B
					bucketB[origin_gain[j]].erase(cell_vector[tmp_net->cell_list[j]]->ptr);
					cell* insert_cell = new cell();
					insert_cell->name = cell_vector[tmp_net->cell_list[j]]->name;
					bucketB[(cell_vector[tmp_net->cell_list[j]])->gain].push_front(insert_cell);
					cell_vector[tmp_net->cell_list[j]]->ptr = bucketB[cell_vector[tmp_net->cell_list[j]]->gain].begin();
				}
			}
			
			
		}
		partA_size -= tmp_cell->size;
		partB_size += tmp_cell->size;
		partA_num--;
		partB_num++;
	}
	else{//from B to A
//	cout << "In condition B\n";
		int n = tmp_cell->net_list.size();
//		cout << "net size: " << n << '\n';
		for(int i = 0; i != n; ++i){
			net* &tmp_net = net_vector[tmp_cell->net_list[i]];
			int m = tmp_net->cell_list.size();
	//		cout << "net name " << tmp_net->name <<" size " << m<<'\n';
			int origin_gain[m];
	//		cout << "og list:\n";
			for(int j = 0; j != m; ++j){
				origin_gain[j] = cell_vector[tmp_net->cell_list[j]]->gain;
		//		cout << cell_vector[tmp_cell->net_list[j]]->name << ' ' << origin_gain[j] << '\n';
			}
//			cout << '\n';
			//before the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->A == 0){
					if(cell_for_update->state == false)	cell_for_update->gain++;
				}
				else if(tmp_net->A == 1){
					if((cell_for_update->partition == 0) && (cell_for_update->state == false)) cell_for_update->gain--;
				}
			}
			//F(n) = F(n) - 1; T(n) = T(n) + 1;
			tmp_net->B--;
			tmp_net->A++;
			tmp_cell->partition = false;
			//after the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->B == 0){
					if(cell_for_update->state == false)	cell_for_update->gain--;
				}
				else if(tmp_net->B == 1){
					if((cell_for_update->partition == 1) && (cell_for_update->state == false)) cell_for_update->gain++;
				}
			}
			
			//adjust location
//			cout << "In some problem\n";
			for(int j = 0; j != m; ++j){
				if(cell_vector[tmp_net->cell_list[j]]->state == true){ continue; }
//				cout << origin_gain[j] << ' ';
				if(cell_vector[tmp_net->cell_list[j]]->partition == false){//in bucket list A
					bucketA[origin_gain[j]].erase(cell_vector[tmp_net->cell_list[j]]->ptr);
					cell* insert_cell = new cell();
					insert_cell->name = cell_vector[tmp_net->cell_list[j]]->name;
					bucketA[(cell_vector[tmp_net->cell_list[j]])->gain].push_front(insert_cell);
					cell_vector[tmp_net->cell_list[j]]->ptr = bucketA[cell_vector[tmp_net->cell_list[j]]->gain].begin();
				}
				else{//in bucket list B
					bucketB[origin_gain[j]].erase(cell_vector[tmp_net->cell_list[j]]->ptr);
					cell* insert_cell = new cell();
					insert_cell->name = cell_vector[tmp_net->cell_list[j]]->name;
					bucketB[(cell_vector[tmp_net->cell_list[j]])->gain].push_front(insert_cell);
					cell_vector[tmp_net->cell_list[j]]->ptr = bucketB[cell_vector[tmp_net->cell_list[j]]->gain].begin();
				}
			}
//			cout << "In some problem\n";
			
		}
		partB_size -= tmp_cell->size;
		partA_size += tmp_cell->size;
		partB_num--;
		partA_num++;
	}

	remove_cell(tmp);
	return;
}

void get_partitionAB_num(){
	int tmp_partA_num = 0;
	int tmp_partB_num = 0;
	for(int i = 0; i < cell_num; ++i){
		if(cell_vector[i]->partition == 0) tmp_partA_num++;
		else tmp_partB_num++;
	}
	partA_num = tmp_partA_num;
	partB_num = tmp_partB_num;
}


int lps_partA_num;
int lps_partB_num;
//bool renew_flag;
int FM_partition(){
	bool flag = false;
	initial_gain();
	get_bucket_list();
	int count = cell_num;
	int free_A = partA_num;
	int free_B = partB_num;
	stack<int> record_cell;
	int sum = 0;
	int largest_partial_sum = 0;
	int iteration = 0;
	int lps_iteration = 0;
	
	while(!flag && count--){
		if(!free_B){
			string tmp_a = get_maxgain_cellname(0);
			if((abs(partA_size - partB_size - 2 * (cell_vector[cell_dictionary[tmp_a]])->size) < constraint)){
				record_cell.push(cell_dictionary[tmp_a]);
				sum += cell_vector[cell_dictionary[tmp_a]]->gain;
				update_gain(tmp_a);
				free_A--;
			}
			else flag = true;
		}
		else if (!free_A){
			string tmp_b = get_maxgain_cellname(1);
			if((abs(partB_size - partA_size - 2 * (cell_vector[cell_dictionary[tmp_b]])->size) < constraint)){
				record_cell.push(cell_dictionary[tmp_b]);
				sum += cell_vector[cell_dictionary[tmp_b]]->gain;
				update_gain(tmp_b);
				free_B--;
			}
			else flag = true;
		}
		else{
			string tmp_a = get_maxgain_cellname(0);
			string tmp_b = get_maxgain_cellname(1);
			if(cell_vector[cell_dictionary[tmp_a]]->gain >= cell_vector[cell_dictionary[tmp_b]]->gain){
				if((abs(partA_size - partB_size - 2 * (cell_vector[cell_dictionary[tmp_a]])->size) < constraint)){
				record_cell.push(cell_dictionary[tmp_a]);
				sum += cell_vector[cell_dictionary[tmp_a]]->gain;
				update_gain(tmp_a);
				free_A--;
				}
				else if((abs(partB_size - partA_size - 2 * (cell_vector[cell_dictionary[tmp_b]])->size) < constraint)){
				record_cell.push(cell_dictionary[tmp_b]);
				sum += cell_vector[cell_dictionary[tmp_b]]->gain;
				update_gain(tmp_b);
				free_B--;
				}
				else flag = true;
			}
			else {
				if((abs(partB_size - partA_size - 2 * (cell_vector[cell_dictionary[tmp_b]])->size) < constraint)){
				record_cell.push(cell_dictionary[tmp_b]);
				sum += cell_vector[cell_dictionary[tmp_b]]->gain;
				update_gain(tmp_b);
				free_B--;
				}
				else if((abs(partA_size - partB_size - 2 * (cell_vector[cell_dictionary[tmp_a]])->size) < constraint)){
				record_cell.push(cell_dictionary[tmp_a]);
				sum += cell_vector[cell_dictionary[tmp_a]]->gain;
				update_gain(tmp_a);
				free_A--;
				}
				else flag = true;
			}
		}
		iteration++;
		
		if(largest_partial_sum <= sum){
//			renew_flag = true;
			largest_partial_sum = sum;
			lps_iteration = iteration;
			lps_partA_num = partA_num;
			lps_partB_num = partB_num;
		}
		
	}
	int pop_num = iteration - lps_iteration;
	//reverse
	for(int i = 0; i != pop_num; ++i){
		int num = record_cell.top();
		cell* &tmp_cell = cell_vector[num];
		tmp_cell->partition = !(tmp_cell->partition);
		record_cell.pop();
	}
	get_AB();
	get_partitionAB_num();
	return largest_partial_sum;
}

void partition_looper(){
	int pass = 0;
	while(1){
		int sum = FM_partition();
		cout << "Pass: " << pass << '\n';
		cout << "LPS: " << sum << '\n';
		if (sum <= 0) break;
		
	}
}

void output_answer(ostream &out){
		get_partitionAB_num();
		out << "cut_size " << cutsize << '\n';
		out << "A " << partA_num << '\n';
		for(int i = 0; i < cell_num; ++i){
			if(cell_vector[i]->partition == 0){
				out << cell_vector[i]->name << '\n';
			}
		}
		cout << "Print A complete\n";
		out << "B " << partB_num << '\n';
		for(int i = 0; i < cell_num; ++i){
			if(cell_vector[i]->partition == 1){
				out << cell_vector[i]->name << '\n';
			}
		}
		cout << "Print B complete\n";
	return;
}

int main(int argc, char *argv[]){
	ios_base::sync_with_stdio(false);
//	std::cin.tie(0);
	
	parse_parameter(argc, argv);
	parse_cell(fh_in_cell);
	fh_in_cell.close();
	parse_net(fh_in_net);
	fh_in_net.close();
	initial_partition();
//	for(vector<cell*>::iterator i = cell_vector.begin(); i != cell_vector.end(); i++)
//			cout << (*i)->name << " " << (*i)->size << ' ' << (*i)->pin << ' ' <<(*i)->state << '\n';
	get_AB();
	get_cutsize();
	get_constraint();
	get_pmax();
//	initial_gain();
//	get_AB();

	partition_looper();
	
//	FM_partition();
//	get_bucket_list();
//	FM_partition();
//	FM_partition();
//	FM_partition();
//	FM_partition();

	get_cutsize();
	cout << "one iteration: " << cutsize << '\n'; 
	output_answer(fh_out);
	fh_out.close();
	return 0;
		
}



