/*
		  Fresh Retro 12 version 0.0-1a

KNOWN BUGS:
	   debug 2 is broken
	   Working on io!	:/
*/

#include <iostream>
#include <vector>
using namespace std; 

string replaceAll(string str, const string &from, const string &to) {
	if(from.empty())
		return str;
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
} //Edited from https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string

int index(string a[30], string find) {
	for (int i=0; i<30; i++) {
		if (a[i] == find) {
			return i;
		}
	}
	return -1;
}

class stack {
	public:
		int sp = 0;
		int pop() {
			int a = data[sp];
			data[sp]=0;
			sp--;
			return a;
		}
		void append(int i) {
			sp++;
			data[sp]=i;
		}
		string display() {
			cout << '[';
			for(int i=1; i<=sp; i++) {
				cout << data[i];
				cout << ((i==sp)? "":", ");
			}
			cout << "]";
			return "";
		}
	private:
		int data[5112];
};

#define MEMORY_SIZE 3500000
int memory[MEMORY_SIZE];
int ip=0;
stack data;
stack address;

class basic_io {
	public:
		/*
		reserved devices:
			  0:generic output
			  1:keyboard
			  2:floating point
			  3:block storage
			  4:unix files
			  5:gopher
			  6:http
			  7:sockets
			  8:unix
			  9:scripting hooks
			 10:random number
		*/
		int type = -1;
		int version = 0;
	void run() {
	}
};

class general_output : basic_io {
	public:
	int type = 0;
	void run() 
	{
		printf("run:%c", data.pop());
		//cout << chr(data.pop());
	}
};


#define IO_DEVICES 2
basic_io IO[IO_DEVICES];

#define DEBUG_MODE 0 //1 for CRC style debuging, 2 for DUSK style debuging

//unpacked vars(for speed/simplicity)
int unpacked0;
int unpacked1;
int unpacked2;
int unpacked3;

void no(int a, int b) {
}

void li(int a, int b) {
	ip++;
	data.append(memory[ip]);
	if (DEBUG_MODE == 2) {
		cout << memory[ip];
	}
}
void du(int a, int b) {
	data.append(a);
	data.append(a);
	if (DEBUG_MODE == 2) {
		cout << a;
	}
}
void dr(int a, int b) {
	if (DEBUG_MODE == 2) {
		cout << a;
	}
}
void sw(int a, int b) {
	data.append(a);
	data.append(b);
	if (DEBUG_MODE == 2) {
		cout << b << ' ' << a << " -> " << a << ' ' << b;
	}
}
void pu(int a, int b) {
	address.append(a);
	if (DEBUG_MODE == 2) {
		cout << a;
	}
}
void po(int a, int b) {
	data.append(address.pop());
	if (DEBUG_MODE == 2) {
		cout << a;
	}
}
void ju(int a, int b) {
	ip=a-1;
	if (DEBUG_MODE == 2) {
		cout << ip;
	}
}
void ca(int a, int b) {
	address.append(ip);
	ip=a-1;
	if (DEBUG_MODE == 2) {
		cout << ip; }
}
void cc(int a, int b) {
	if (b != 0) {
		address.append(ip);
		ip = a-1;
		if (DEBUG_MODE == 2) {
			cout << ip; }
	}
	else if (DEBUG_MODE==2) {
		cout << "condition faied";
	}
}
void re(int a, int b) {
	ip = address.pop();
	if (DEBUG_MODE == 2) {
		cout << ip;
	}
}
void eq(int a, int b) {
	data.append((b==a) ? -1 : 0);
	if (DEBUG_MODE==2) {
		cout << b << "==" << a;
	}
}
void ne(int a, int b) {
	data.append((b!=a) ? -1 : 0);
	if (DEBUG_MODE==2) {
		cout << b << "!=" << a;
	}
}
void lt(int a, int b) {
	data.append((b<a) ? -1 : 0);
	if (DEBUG_MODE==2) {
		cout << b << "<" << a;
	}
}
void gt(int a, int b) {
	data.append((b>a) ? -1 : 0);
	if (DEBUG_MODE==2) {
		cout << b << ">" << a;
	}
}
void fe(int a, int b) {
	if (a<0) {
		switch (a) {
			case -1: data.append(data.sp+1); break;
			case -2: data.append(address.sp+1);break;
			case -3: data.append(MEMORY_SIZE);break;
			case -4: data.append(2147483647);break;
			case -5: data.append(-2147483648);break;
		}
		return;
	}
	data.append(memory[a]);
	if (DEBUG_MODE==2) {
		cout << memory[a] << " from " << a;
	}

}
void st(int a, int b) {
	memory[a] = b;
	if (DEBUG_MODE == 2) {
		cout << b << " into " << a; }
}
void ad(int a, int b) {
	data.append(b+a);
	if (DEBUG_MODE==2) {
		cout << b << "+" << a << "=" << a+b;
	}
}
void su(int a, int b) {
	data.append(b-a);
	if (DEBUG_MODE==2) {
		cout << b << "-" << a << "=" << a-b;
	}
}
void mu(int a, int b) {
	data.append(b*a);
	if (DEBUG_MODE==2) {
		cout << b << "*" << a << "=" << a*b;
	}
}
void di(int a, int b) {
	data.append(b%a);
	data.append(b/a);
	if (DEBUG_MODE==2) {
		cout << b << "/%" << a << "=" << b/a << ' ' << b%a;
	}
}
void an(int a, int b) {
	data.append(b&a);
	if (DEBUG_MODE==2) {
		cout << b << "&" << a << "=" << (a&b);
	}
}
void Or(int a, int b) {
	data.append(b|a);
	if (DEBUG_MODE==2) {
		cout << b << "|" << a << "=" << (a|b);
	}
}
void xo(int a, int b) {
	data.append(b^a);
	if (DEBUG_MODE==2) {
		cout << b << "^" << a << "=" << (a^b);
	}
}
void sh(int a, int b) {
	if (a < 1) {
		data.append(b<<(0-a));
		return;
	}
	data.append(b>>a);
	if (DEBUG_MODE==2) {
		cout << b << "by" << a;
	}
}
void zr(int a, int b) {
	if (a == 0) {
		ip = address.pop();
		if (DEBUG_MODE==2) {
			cout << ip;
		}
		return;
	}
	data.append(a);
	if (DEBUG_MODE==2) {
		cout << "condition failed";
	}
}
void ha(int a, int b) {
	ip = MEMORY_SIZE+5;
}
void ie(int a, int b) {
	data.append(IO_DEVICES);
}
void iq(int a, int b) {
	//As I have my IO devices in order, to get it to work with my hack I am cheating here
	data.append(0);//data.append(IO[a].version);
	data.append(a);//data.append(IO[a].type);
}
void ii(int a, int b) {
	switch (a) {
		case 0:
			cout << (char)data.pop();
			break;
		case 1:
			char ch;
			ch = cin.get();
			data.append((int)ch);
		default:
			break;
	}
}

typedef void (*Handler)(int, int);

Handler op[30] = {
	no,
	li,
	du,
	dr,
	sw,
	pu,
	po,
	ju,
	ca,
	cc,
	re,
	eq,
	ne,
	lt,
	gt,
	fe,
	st,
	ad,
	su,
	mu,
	di,
	an,
	Or,
	xo,
	sh,
	zr,
	ha,
	ie,
	iq,
	ii
};
string op_names[30] = {
	"no","li","du","dr","sw","pu","po",
	"ju","ca","cc","re","eq","ne","lt",
	"gt","fe","st","ad","su","mu","di","an",
	"Or","xo","sh","zr","ha","ie","iq","ii"
};

int index(string s) {
	for (int i = 0; i<30; i++) {
		if (op_names[i] == s) {
			return i;
		}
	}
	return 0;
}

int pack(string l) {
	l=l.append("nononono");
	l=replaceAll(l,"..","no");
	int a=index(l.substr(0,2));
	int b=index(l.substr(2,2));
	int c=index(l.substr(4,2));
	int d=index(l.substr(6,2));
	return (a<<0)+(b<<8)+(c<<16)+(d<<24);
}
void unpack(int instr) {
	unpacked0=(instr&0x000000FF)>>0;
	unpacked1=(instr&0x0000FF00)>>8;
	unpacked2=(instr&0x00FF0000)>>16;
	unpacked3=(instr&0xFF000000)>>24;
}

bool contains(int instr, int *selection, int checkLength) {
	for (int i=0; i<checkLength; i++) {
		if (instr == selection[i]) {
			return true;
		}
	}
	return false;
}
void excecute(int instr) {
	if (DEBUG_MODE==1) {
		cout << ip << ' ' << op_names[instr] << " \n " << data.display() << " \n " << address.display() << "\n\n";
	}
	if (DEBUG_MODE==2) {
		cout << op_names[instr] << ' ';
	}
	int two[15] = {4,9,11,12,13,14,16,17,18,19,20,21,22,23,24};
	int zero[6] = {0,1,6,10,26,27};
	int one[9]  = {2,3,5,7,8,15,25,28,29};
	if (contains(instr, two, 15)) {
		int a=data.pop();
		int b=data.pop();
		op[instr](a,b);
	} 
	else if (contains(instr, zero, 6)) {
		op[instr](0,0);
	}
	else if (contains(instr, one, 9)) {
		op[instr](data.pop(),0);
	}
	else {
		cout << "invalid instruction " << instr << " in " << ip << "\n";
		ip=MEMORY_SIZE+5;
	}
	if (DEBUG_MODE==2) {
		cout << "\ndata:   " << data.display() << "\n";
		cout << "address: " << address.display() << "\n\n";
	}
}

void run() {
	while ((ip < MEMORY_SIZE) and (ip >= 0)) {
		int instr = memory[ip];
		unpack(instr);
		if (DEBUG_MODE) {
			cout << "===============\n";
		}
		if (DEBUG_MODE==2) {
			cout << "ip:" << ip << '	' << instr << '\n';
			string fu = op_names[unpacked0];
			fu=fu.append(op_names[unpacked1]).append(op_names[unpacked2]).append(op_names[unpacked3]);
			cout << "commands: " << replaceAll(fu, "no","..") << '\n';
		}
		excecute(unpacked0);
		excecute(unpacked1);
		excecute(unpacked2);
		excecute(unpacked3);
		ip++;
	}
}

void patch(vector<int> o, int start) {
	for (int i=0; i<o.size(); i++) {
		memory[i+start] = o[i];
	}
;}

int main() {
	vector<int> l = {
};
}
