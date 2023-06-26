#include<iostream>
#include<fstream>
#include<iomanip>
#include<algorithm>
#include<stack>
#include<queue>
#include<vector>
#include<map>
#include<set>
#include<string>
#include<cstring>
#include<utility>
#include<regex>
#include<sstream>
using namespace std;

//efficient construction of LALR(1) Table

//LR1 item = LR0 item(core) + lookahead
//规定输入每个token中间都要加空格
//A -> a A | B b | d
//和->,| 之间也加空格
//Data Structure 沿用LL1

set<string> non_terminal;
set<string> terminal;

// data structure of grammar
vector<string> gram; //grammar with raw productions

//grammar after preprocess
map<string, vector<string> > grammar; //前面是Nt,后面有多个choice string (if it has)

string startSymbol; //开始符号，定为输入第一个产生式的左边 拓广文法后被改变

const string EOI="$";
const string NIG="#"; //a symbol not in grammar, used in determining lookaheads (propagated or spontaneous)
const string EPS="&"; //epsilon 用&代替ε

void split_pro() //拆分产生式
{
	for (auto p:gram)
	{
		string lhs,rhs;

		for (int i=0;i<p.size()-1;i++)
		{
			if (p[i]=='-' && p[i+1]=='>')
			{
				lhs=p.substr(0,i-1);  //update here cuz the white space before ->
				rhs=p.substr(i+3); //para_2nd default=npos 不加第二个参数拷贝到string末尾 i.e. substr(i+2, string::npos) //update here cuz the white space after ->
				break;
			}
		}
		//rhs 开头没有空格
		int start=0,length=0; //indicate positions to split production contains | 每次substr的param 切分的位置和长度
		for (int i=1;i<rhs.size()-1;i++) //split | on rhs
		{
			if (rhs[i-1]==' ' && rhs[i]=='|' && rhs[i+1]==' ') //delimiter changed from '|' to 'space|space'
			{
				length=i-start-1;
				grammar[lhs].push_back(rhs.substr(start,length));
				start=i+2; //更新开始位置
			}
		}
		grammar[lhs].push_back(rhs.substr(start)); //循环结束后，最后还剩一个从start标的位置截取到末尾，如果产生式里没出现|，start还是0，从头截取到尾，string rhs不变
	}
}

vector<string> split_str(string str) //从string s 里提取用空格分离的每个符号
{ //先split_pro()拆分产生式 才能用split_str() 和提取NT， 因为split_str默认string是没有| 的
	vector<string> tokens;
	string s=str; //copy string副本
	string delimiter=" ";
	int pos=0;
	while ((pos=s.find(delimiter))!=string::npos)
	{
		string token=s.substr(0,pos);
		tokens.push_back(token);
		s.erase(0,pos+delimiter.length()); //把前面提取过的部分删了
	}
	tokens.push_back(s);

	return tokens;
}

string merge_str(vector<string> tokens) //把一个vector string间加上空格，合成一个完整的string ，两头没加空格
{
	string res=tokens[0];
	for (int i=1;i<tokens.size();i++)
		res+=" "+tokens[i];
	return res;
}

void get_symbols() //提取N和T集合
{
	for (int x=0;x<gram[0].size()-1;x++) //get start symbol
		if (gram[0][x]=='-' && gram[0][x+1]=='>')
		{
			startSymbol=gram[0].substr(0,x-1); //update here cuz the white space before ->
			break;
		}


	for (auto p:gram) // Gather all non terminals
	{
		for (int i=0;i<p.size()-1;i++)
		{
			if (p[i]=='-' && p[i+1]=='>')
			{
				non_terminal.insert(p.substr(0,i-1)); //update here cuz the white space before ->
				break;
			}
		}
	}

	for (auto line:grammar) // Gather all terminals
	{ //注意是在split_pro后的grammar遍历，不是原字符串gram
		for (auto p:line.second)
		{
			//要先split_pro 才能用split_str 因为split_str默认string是没有| 的
			vector<string> tokens = split_str(p);
			for (auto token:tokens)
			{
//				cout<<"@"<<token<<"@"<<endl;//debug
				if (token!="&" && token!="" && token.size()>0 && non_terminal.find(token) == non_terminal.end()) //不为ε(&) 且 不在non_terminal set中
					terminal.insert(token); //加入terminal set
			}
		}
	}
}

bool isTerminal(string token)
{
	if (terminal.find(token)!=terminal.end())
		return true;
	return false;
}

bool isNonTerminal(string token)
{
	if (non_terminal.find(token)!=non_terminal.end())
		return true;
	return false;
}

map<string, set<string> > First; //map<nonterminal, its first set>

map<string, set<string> > Follow; //map<nonterminal, its follow set>

set<string> getStrFirst(string s) //get first set of a specific string //注意这里需要特殊处理 $ 串 和 s is empty的情况，和LL里不一样
{
	set<string> strFirst;

	if (s.empty() || s=="")
	{
		strFirst.insert(EPS);
		return strFirst;
	}

	if(s==EOI)  //First($) = {$} //lookahead合并后s不会出现$
	{
		strFirst.insert(EOI);
		return strFirst; //结束
	}

	vector<string> tokens=split_str(s);
	int epsilon_num=0; //统计能变成空的Nonterminal个数

	for (auto token:tokens)
	{
		if (token=="&" && tokens.size()==1) //如果要求的字符串就只有ε
		{
			strFirst.insert("&");
			return strFirst; //加完空 直接结束
		}

		int flag=0; //假设不用往后再看了（当前token是terminal,或当前token是nonterminal但不能变成空）
		if (isTerminal(token))
		{
			strFirst.insert(token);
			break;
		}
		else if (isNonTerminal(token))
		{
			//First[token] 是token 的first set
			for (auto x:First[token])
				if (x!="&") //去掉ε加入当前str的first set
					strFirst.insert(x);
				else //x==&
				{
					flag=1;
					epsilon_num++;
				}

		}

		if(!flag) //当前的non-terminal不能变成ε，就不用往后看了。
			break;
	}
	if (epsilon_num==tokens.size()) //从头到尾每一个都能变成空，说明整个串也能推出空
		strFirst.insert("&");
	return strFirst;
}

void getFirst() // get first sets of all non_terminals
{
	map<string, set<string> > oldFirst; //保存上一轮遍历的First
	while (true)
	{
		oldFirst=First;  //上一轮遍历的First 存入oldFirst 这一轮更新First 结束后对比

		for (auto it:grammar) //对所有产生式遍历一遍
		{
			string curNt = it.first;
			for (auto str:it.second)
			{
				set<string> addFirst = getStrFirst(str);
				First[curNt].insert(addFirst.begin(),addFirst.end());
			}
		}

		if (oldFirst==First) //直到整个First没有变化的时候结束
			break;
	}
}

void PrintFirst()
{
	for (auto n:First)
	{
		cout<<"First("<<n.first<<") : ";
		for (auto symbol:n.second)
		{
			cout<<symbol<<"  ";
		}
		cout<<endl;
	}
 }

set<string> getFollow_ofN(string N) //get follow sets of one specific Nonterminal
{
	set<string> FollowN;
	for (auto it:grammar ) //对每个产生式
	{
		int flag=0; //先假设不需要加左边的follow集合进去
		for (auto str:it.second) //str是每个产生式右边的字符串
		{
			int pos=0;
			while ((pos = str.find(N))!=string::npos) //int pos = str.find(N); //在右边找N的位置  //	if(pos!=-1) //如果找到N
			{ //N后面的子串的First 集合减去ε 加入Follow(N)里

				if (pos+N.size()>=str.size()) //如果在最后找到N，后面没有跟任何字符，(其实就相当于后面跟的串的first set里有空)，也要把产生式左边符号的Follow 集合加到N的Follow集合里
				{
					flag=1;
					break; //也不用再往后看了，str已经到头了
				}
				string afterN = str.substr(pos+N.size()+1); //空格
				set<string> First2add=getStrFirst(afterN); //N后面的子串的First 集合
				for (auto x:First2add)
				{
					if (x!="&") //去掉ε
						FollowN.insert(x);
					else // if (x=="&") N后面的子串的First 集合有空
						flag=1; //需要把it.first 也就是这个产生式左边的符号 的Follow 集合加到N的Follow集合里
				}
			 	str=afterN; //更新str 前面找过的部分去掉，留后面没找的部分再继续找有没有N
			}
		}
		if(flag) //产生式左边符号的Follow 集合加到N的Follow集合里
			FollowN.insert(Follow[it.first].begin(), Follow[it.first].end());
	}
	return FollowN;
}

void getFollow()
{
	Follow[startSymbol].insert("$"); //开始符号后面一定有终结符$
	map<string, set<string> > oldFollow; //保存上一轮遍历的Follow
	while (true)
	{
		oldFollow=Follow;

		for (auto it:grammar ) //对每个产生式it
		{
			for (auto str:it.second) //str是每个产生式右边的字符串
			{
				vector<string> tokens=split_str(str); //每个产生式右边拆成一个一个符号
				for (auto token:tokens) //对每一个符号
				{
					if (isNonTerminal(token)) //如果是非终结符就算一遍它的follow集合 再加到对应的follow集合里
					{
						set<string> addFollow = getFollow_ofN(token);
						Follow[token].insert(addFollow.begin(),addFollow.end());
					}
				}
			}
		}
		//扫完一轮
		if (oldFollow==Follow) //直到整个Follow没有变化时结束
			break;
	}
}

void PrintFollow()
{
	for (auto n:Follow)
	{
		cout<<"Follow("<<n.first<<") : ";
		for (auto symbol:n.second)
		{
			cout<<symbol<<"  ";
		}
		cout<<endl;
	}
}

void augmentedGrammar() //拓广文法
{
	string start2=startSymbol+"'";
	non_terminal.insert(start2); //update NT set
	vector<string> production;
	production.push_back(startSymbol);
	grammar[start2]=production;
	startSymbol=start2; //update the start symbol to the new one
}


// LR(0) item, production with a dot. (currently with no lookahead component //e.g.: S -> a .B , a
class item
{
	private:
		string left;
		string right;
		int dotPos;  //the position of the dot (before which symbol with no space, metric is number of token,not char, start 0 in right hand sight)
//LR1 items
//		string lookahead; //搜索符，没有合并，多个搜索符就是多个item
		set<string> lookahead; //合并搜索符
//to deal with lookaheads propagations
//1. propagation of lookaheads
//2. spontaneously generated lookaheads for kernels in each state
		//spontaneous 直接存到item的lookahead vector中，作为INIT
		vector<pair<int,int> > propagation; //propagate to which state and which item <state index,item index> //obtain item index via vector iterator-begin()


	public:
		item(string left="", string right="", int dotPos=0); //, string lookahead="" ); //带参数的构造函数 参数设置默认值 lookahead="#" is used to compute propagation of lookaheads
		string toString(); //return production string with dot
		string toOriginalString(); //return production string without dot

		string getLeft();
		string getRight();
		int getDotPos();
		set<string> getLookahead();

		void increaseDotPos(); //dot 后移一位
		string getSymbolRight2Dot();  // return the symbol immediately to the right of the dot in current produtcion
		bool isComplete(); //if this item is complete item (i.e. dot is on the right end)
		bool isAccept(); //especially, if this item is S'->S.
		bool isEpsilon(); //判断item 产生式右侧是否为ε

		string str2Lookahead(); //如果当前item,dot后面是Nonterminal B，返回当前item在B后面的整体字符串(不包括lookahead) 末尾无空格

		//运算符重载 overload  不考虑lookahead
		bool operator==(const item& right) const
		{
      		return this->left == right.left && this->right==right.right && this->dotPos== right.dotPos; // && this->lookahead == right.lookahead;
  		}
	  	bool operator!=(const item& right) const
		{
	      	return this->left != right.left && this->right!=right.right && this->dotPos != right.dotPos; // && this->lookahead != right.lookahead;
	  	}

	  	//这个<重载如果有问题会导致状态数不对 //分成多层写
	  	bool operator<(const item& right) const  //set insert need this
		{
			if (this->left!=right.left)
				return this->left < right.left;
			else // this->left==right.left
				if (this->right!=right.right)
					return this->right < right.right;
				else //this->right == right.right
//					if (this->dotPos!=right.dotPos)
						return this->dotPos < right.dotPos;
//					else //this->dotPos == right.dotPos
//						return this->lookahead < right.lookahead;
	  	}

 		friend vector<item> closure1(vector<item> I);
 		friend void getCanonicalCollection();
 		friend void DeterminingLookaheads();
 		friend bool isNextItem(item i1, item i2); //判断i2是否为i1的next，就是DotPos往后移一个，不看lookahead
 		friend void PropagateLookaheads();
		friend void DebugPrint();

};

//// LR(1) item, production with a dot. and a lookahead component //e.g.: S -> a .B , a
//class item1:protected item  //inherited from class item
//{
//	private:
//		string lookahead; //搜索符，没有合并，多个搜索符就是多个item
//
//	public:
//		item1(string left="", string right="", int dotPos=0, string lookahead=""):item(left,right,dotPos) //构造函数，初始化列表调用基类构造函数
//		{
//			this->lookahead=lookahead;
//		}
//
//		string getLookahead();
//		void setLookahead(string x);
//
//		string str2Lookahead(); //如果当前item,dot后面是Nonterminal B，返回当前item在B后面的整体字符串(包括lookahead)
//};

item::item(string left, string right, int dotPos)  //, string lookahead)
{
	this->left=left;
	this->right=right;
	this->dotPos=dotPos;
//	this->lookahead=lookahead;
}

string item::toString()
{
	vector<string> tokens = split_str(right);
	if (dotPos>tokens.size())
	{
		cerr<<"Error: Invalid Dot Position! \ntoString() Failed."<<endl;
		exit(0);
	}
	string res= left + " -> ";
	for (int i=0;i<tokens.size();i++)
	{
		if (dotPos==i)
			res+="."+tokens[i]+" ";
		else if (i!=tokens.size()-1)
			res+=tokens[i]+" ";
		else if (i==tokens.size()-1)
			res+=tokens[i]; //末尾不加空格
	}
	if (dotPos==tokens.size()) //complete item, dot is in the right end
	res+=".";

//	string res=+ right.substr(0,dotPos) + "." + right.substr(dotPos);

	res+= " ,";// "+lookahead;  // A -> a .b , c
	for (auto la:lookahead)
		res+=" "+la;

	return res;
}

string item::toOriginalString()
{
	return left + " -> " + right;
}

string item::getLeft()
{
	return left;
}

string item::getRight()
{
	return right;
}

int item::getDotPos()
{
	return dotPos;
}

set<string> item::getLookahead()
{
	return lookahead;
}

void item::increaseDotPos()
{
	dotPos++;
}

string item::getSymbolRight2Dot()
{
    vector<string> tokens = split_str(right);

	if (dotPos==tokens.size()) //complete item
		return ""; //点在最后，后面没有符号了，表明可以规约，返回空 //具体判断是否规约封装成另一个函数了
	if (dotPos>tokens.size())
	{
		cerr<<"Error: Invalid Dot Position! \ngetSymbolRight2Dot() Failed."<<endl;
		exit(0);
	}
	return tokens[dotPos];
}

string item::str2Lookahead() //如果当前item,dot后面是Nonterminal B，返回当前item在B后面的整体字符串(不包括lookahead) 最后不带空格 //如果B后面没有字符了，res就是empty
{
	string res;
	string nextSymbol=getSymbolRight2Dot();
	vector<string> tokens = split_str(right);
	if (isNonTerminal(nextSymbol) )
	{
		for (int i=dotPos+1; i<tokens.size()-1; i++)
			res+=tokens[i]+" ";
		if (dotPos+1<tokens.size())
			res+=tokens[tokens.size()-1]; //最后一个特殊处理，末尾不加空格
	}
	return res;
}

bool item::isComplete()
{
	vector<string> tokens = split_str(right);
	if (dotPos==tokens.size())
		return true;
	return false;
}

bool item::isAccept()
{
	if (this->left==startSymbol && this->isComplete() )
		return true;
	return false;
}

bool item::isEpsilon()
{
	if (this->right == "&")
		return true;
	return false;
}

bool isNextItem(item i1, item i2) //判断i2是否为i1的next，就是DotPos往后移一个，不看lookahead
{
	if (i1.getLeft() == i2.getLeft() && i1.getRight() == i2.getRight() && i1.getDotPos()+1 == i2.getDotPos())
		return true;
	return false;
}

//closure for LR(0) items, return a set of items
vector<item> closure0(vector<item> I)  //param: a set of items for grammar G'  // maybe need to deal with I is empty?
{
	vector<item> J=I;
	vector<item> lastJ; //store J on last one round to check if there are new items added into J

	map<string, bool> added; //<NT,if it is added to the set>
	for (auto NT:non_terminal) //initialize added
		added[NT]=false;
	//added[B] is set to true if and when we add the item B->.r for each B-production

	do
	{
		lastJ = J;
		for (int it=0;it<J.size();it++) //迭代器循环可能导致内存失效，改为下标循环 下面所有it改为 J[it] //for (item it:J)  //for each item in J
		{
			string NextSymbol=J[it].getSymbolRight2Dot();
//			cout<<"Symbol after dot: "<<NextSymbol<<endl; //debug
			if ( isNonTerminal(NextSymbol) )  //if the symbol B on the immediate right of dot is NT
			{
				if (!added[NextSymbol]) //if this production is not in J
				{
					//add it to J
					for ( auto r: grammar[NextSymbol] ) //for each production B -> r of G
					{
						item newItem(NextSymbol, r, 0);
//						cout<<"newItem: "<<newItem.toString()<<endl; //debug
						J.push_back(newItem); //need to overload operator <
					}
					added[NextSymbol]=true;
				}
			}
		 }
	} while (lastJ.size() != J.size());  //lastJ!=J //no more items are added on one round

	return J;
}

//closure for LR(1) items, return a set of items
vector<item> closure1(vector<item> I)  //param: a set of items for grammar G'  // set<items_type> I
{
	getFirst(); //先求出First集合备用

	vector<item> J=I;
	vector<item> lastJ; //store J on last one round to check if there are new items added into J

////lookahead已合并处理，但added不能沿用LR0
	//这里added要判断NT和lookahead string 都没加过才行
	map< pair<string,string >, bool> added; //<NT,str_LA> ,if it is added to the set>
	//关于initialize added，没加过就是空，find找不到，加过就是true

	//如果added出问题的话就会导致closure()不对

	do
	{
		lastJ = J;
		for (item it:J)  //for each item in J
		{
			string NextSymbol=it.getSymbolRight2Dot();
//			cout<<"Symbol after dot: "<<NextSymbol<<endl; //debug
			if ( isNonTerminal(NextSymbol) )  //if the symbol B on the immediate right of dot is NT
			{
				string str_LA = it.str2Lookahead();
//				cout<<"lookahead<"<<str_LA<<">"<<endl; //debug

				if ( added.find(make_pair(NextSymbol, str_LA)) == added.end() ) //!added[NextSymbol] if this production is not in J
				{//add it to J
					for ( auto r: grammar[NextSymbol] ) //for each production B -> r of G
					{
						item newItem(NextSymbol, r, 0);
						auto exist = find(J.begin(),J.end(),newItem);
						if (exist!=J.end()) //如果newItem已经在J里存在，更新其lookahead vector即可 (*exist).lookahead
						{
							set<string> First_set = getStrFirst(str_LA);
							if (First_set.find(EPS)!=First_set.end()) //First set里面有Epsilon，lookahead set添加到扩展项目set里，否则就用不到lookahead
								for (auto la:it.lookahead)
									(*exist).lookahead.insert(la);
							//把first set里除了epsilon的symbol 加入lookahead vector
							for (auto t:First_set) //for each terminal b in First(Ba)
							{
								if (t!=EPS)
									(*exist).lookahead.insert(t);
							}
						}
						else //newItem not in J
						{
							set<string> First_set = getStrFirst(str_LA);
							if (First_set.find(EPS)!=First_set.end()) //First set里面有Epsilon，lookahead set添加到扩展项目set里，否则就用不到lookahead
								for (auto la:it.lookahead)
									newItem.lookahead.insert(la);
							//把first set里除了epsilon的symbol 加入lookahead vector
							for (auto t:First_set) //for each terminal b in First(Ba)
							{
								if (t!=EPS)
									newItem.lookahead.insert(t);
							}
							J.push_back(newItem); //need to overload operator <
						}
					}
					added[make_pair(NextSymbol, str_LA)]=true;
				}
			}
		 }
	} while (lastJ != J);  //lastJ!=J //no more items are added on one round

	return J;
}


class state //SetOfItems
{
	private:
		static int num; //计数
		int index;//状态编号
		vector<item> KernelItems; //主要是看 kernel items
		vector<item> AllItems;
		mutable map<string, int> transitions; //goto sets  //this state via <string> X to <int>index state  //maybe set to public??
		//相当于用transitions 代替了龙书里goto()的功能 在getcanonical过程中求出来的 单独写成一个函数有点困难
	public:
		state();
		state(vector<item> KernelItems, vector<item> AllItems); //考虑把AllItems去掉，除了规约epsilon的时候都用不到
//		Goto();
		int getIndex();

		//overload operator for set.insert
		bool operator==(const state& right) const
		{
      		return this->KernelItems == right.KernelItems; //用kernelItems,不用index
  		}
	  	bool operator!=(const state& right) const
		{
	      	return this->KernelItems != right.KernelItems;
	  	}
	  	bool operator<(const state& right) const  //better ways?
		{
	      	return this->KernelItems < right.KernelItems;
	  	}

		friend void getCanonicalCollection(); //友元函数
		friend void DeterminingLookaheads(); //求出each item对应的 propagate from ? to ? 关系  和 spontaneous set 存入lookahead set中
		friend void constructACTIONandGOTO();
		friend void PropagateLookaheads();
		friend void DebugPrint();
};

int state::num=0; //静态成员变量必须初始化 且在类外部初始化

state::state()
{
	index=num;
	num++;

//	initialize all transitions not exist as -1 as a flag
	for (auto X:non_terminal)
		this->transitions[X]=-1;
	for (auto Y:terminal)
		this->transitions[Y]=-1;
}

state::state(vector<item> KernelItems, vector<item> AllItems)
{
	index=num;
	num++;
	this->KernelItems=KernelItems;
	this->AllItems=AllItems;

//	initialize all transitions not exist as -1 as a flag
	for (auto X:non_terminal)
		this->transitions[X]=-1;
	for (auto Y:terminal)
		this->transitions[Y]=-1;

}

int state::getIndex()
{
	return index;
}

vector<item> Goto (vector<item> I, string X) //I is a set of items and X is a grammar symbol, GOTO(I,X) specifies the transition from the state for I under input X
{ //the GOTO function is used to de ne the transitions in the LR(0) automaton for a grammar
	vector<item> nextI;
	for (auto it: I)
	{
		if (it.getSymbolRight2Dot() == X )  //a.Xb
		{
			item nextItem(it.getLeft(), it.getRight(), it.getDotPos()+1); // aX.b  //in next State as kernel item
			if (find(nextI.begin(),nextI.end(),nextItem)==nextI.end()) //去重
				nextI.push_back(nextItem);
		}
	}
	//so far we got the kernel items of next state(SetOfItems)
	//then we just need to apply closure() on nextI we get the whole next state.
	//but i decided not to do it yet.
	return nextI;
}

//the canonical collection is like C = {I0, I1, I2, ... In} where Ix indicates a set of items (a state in FSM) and computed from closure(S'->S) and GOTO()
//if we had canonical collection C and transition function GOTO(I,X) , we got the DFA of LR(0), can derive a LR(0) table based on this.
//set<state> C; //global
//set元素无法修改，无法进行lookahead computation loop 修改state item的lookahead,改用vector+去重判断
vector<state> C; //global //C的index应该和state的index是对应符合的
//compute the canonical collection of sets of LR(0) items
void getCanonicalCollection()  //maybe return set<state>
{
	item i0(startSymbol, grammar[startSymbol][0], 0);
	//spontaneous 直接存到item的lookahead vector中，作为INIT
	i0.lookahead.insert(EOI); //单独处理，I0's INIT spontaneous lookahead is EOI($)

	vector<item> I0 = { i0 }; //I0 is originally set to the augmented grammar S' -> .S
	//kernel items: I0
	//all items: closure(I0)

	state S0 = state(I0, closure0(I0));  //调用构造函数时num自动++
//	set<state>
	C = {S0};
//	vector<state> lastC;
	int lastCsize;

	do
	{
		lastCsize=C.size();

		//用迭代器可能内存失效，换成下标遍历vector //for (auto &S:C) //for each set of items (states) in C //要引用 因为需要修改S的成员变量transitions，mutable解决编译报错
		for (int j=0;j<C.size();j++) //用C[j]替换所有S
		{
			vector<item> I=C[j].AllItems;

//			//print to debug
//			cout<<C[j].index<<endl;
//			for (auto bbb:C[j].AllItems)
//			cout<<bbb.toString()<<endl;
//			cout<<endl;

//			set<item> kernelItems = S.KernelItems;

			//for each grammar symbol X, here I need to tranverse T and NT set respectively, we union the two sets as symbols
			set<string> symbols;
			set_union(non_terminal.begin(), non_terminal.end(), terminal.begin(), terminal.end(), std::inserter(symbols, symbols.begin()));

			for (auto X:symbols)
			{
				vector<item> next_kItems = Goto(I,X); //my Goto() returns next state's kernel items before closure()

//				//print to debug
//				for (auto kkk:next_kItems)
//				cout<<"Via "<<X<<" GOTO: "<<endl<<kkk.toString()<<endl;

				if (!next_kItems.empty()) //not empty //只要不空就说明有这样一个transition指出去，要存下这个边才行
				{
					state nextS(next_kItems, closure0(next_kItems)); //num++ automatically
					auto iter= find(C.begin(),C.end(),nextS);
					if (iter!=C.end()) //this new state is already in set C
					{
						state::num--; //编号减回去
						C[j].transitions[X] = (*iter).index;  //save transition
					}
					else //not in C
					{
						C[j].transitions[X]=nextS.index; //save state's transition
//						cout<<C[j].index<<"--"<<nextS.index<<endl; //debug
						C.push_back(nextS); //add new state to C
					}
				}
			}
			//end of traverse of symbols
		}
	} while (lastCsize!=C.size());


//伪代码
//	set<state> C = {clousure(I0)};  // I0 = {S'->.S}
//	set<state> lastC;
//	do
//	{
//		lastC = C
//		for ()  each set of items I in C
//		{
//			for () each grammar symbol X //here i need to tranverse T and NT set respectively
//			{
//				if () Goto(I,X) is not empty and not in C
//				{
//					add Goto(I,X) to C //here Goto(I,X) should returns a set of items after closure(), but mine is before closure() attention!!
//				}
//			}
//		}
//
//	}while (lastC!=C); //until no new sets of items are added to C on a round

//	return C; //if needed

// print to test
	cout<<"TOTAL DFA States: "<<state::num<<endl<<endl;
	cout<<"DFA Transitions:"<<endl;
	for (auto ss:C)
	{
		for (auto x:non_terminal)
		if (ss.transitions[x]!=-1)
			cout<<ss.index<<" --  "<<x<<"  --> "<<ss.transitions[x]<<endl;
		for (auto x:terminal)
		if (ss.transitions[x]!=-1)
			cout<<ss.index<<" --  "<<x<<"  --> "<<ss.transitions[x]<<endl;
	}

}

//now we got the canonical collections of sets of LR(0) items, that is, we got the kernels of the sets of LR(0) items for G.
//then we need to apply the Determining-Lookaheads-Algorithm to these kernels with each symbol X.

void DeterminingLookaheads()
{
	for (auto &S:C) //for each state in C , (sets of LR0 items)
	{
		for (auto &itm:S.KernelItems) //for each item A->a.C in K
		{
			item tmp = item(itm.getLeft(),itm.getRight(),itm.getDotPos());
			tmp.lookahead.insert(NIG); // create a new temp item A->a.C,#
			vector<item> vtmp = {tmp};
			vector<item> J = closure1(vtmp); //J = closure( { [A->a.C,#] } )

//			cout<<"[# item in J] "<<endl;

			for (auto B:J) // B->r.Xg,a is in J  //each item B (with lookahead) in J
			{
//				cout<<B.toString()<<endl;//debug
				if (B.isComplete() || B.isEpsilon())  //B is complete item or epsilon, dot在最后，没有下一个状态
					continue;

				string X = B.getSymbolRight2Dot();
				int s_index = S.transitions[X]; //下一个状态编号
				int i_index; //存 B->rX.g in GOTO(I,X) 在next state的kernelItems vector中的index

//				cout<<"[state]"<<S.index<<" --- "<<X<<"--- "<<s_index<<endl; //debug

				for(auto la: B.getLookahead() )
				{
					if (la!= NIG) //a is not #, conclude that lookahead is generated spontaneously for item B->rX.g in GOTO(I,X)
					{ //找到下一个item在哪个state的什么位置，做spontaneous记录
						for (auto it = C[s_index].KernelItems.begin(); it !=C[s_index].KernelItems.end(); ++it )//C[s_index]通过vector下标访问下一个state
						{
							if (isNextItem(B,*it)) //find item B->rX.g in GOTO(I,X)
								(*it).lookahead.insert(la); //spontaneous 改变原有的lookahead 作为INIT
						}
					}
					else if (la==NIG) // a is #, conclude that lookaheads propagate from A->a.C in I to B->rX.g in GOTO(I,X)
					{ //对itm的propagation 做记录，记录会传播到哪些state的哪些item
						for (auto it = C[s_index].KernelItems.begin(); it !=C[s_index].KernelItems.end(); ++it )//C[s_index]通过vector下标访问下一个state
						{
							if (isNextItem(B,*it)) //find item B->rX.g in GOTO(I,X)
								i_index = distance( C[s_index].KernelItems.begin(), it); // 计算迭代器与vector.begin()之间的距离
						}

						itm.propagation.push_back(make_pair(s_index,i_index)); //要改变itm.propagation存储的值
					}
				}

			}
		}

	}
}

//void DebugPrint()
//{
//	cout<<endl<<endl<<"LOOKAHEAD RESULT:"<<endl<<endl;
//	for (auto S:C)
//	{
//		cout<<S.getIndex()<<endl;
//		for (auto I : S.KernelItems )
//		{
//			cout<<I.toString()<<endl;
//			if (I.propagation.empty() || I.propagation.size()==0);
//			else
//			{
//				cout<<"Propagation: "<<endl;
//				for (auto p: I.propagation)
//				{
//					cout<<"[state]"<<p.first<<"  [item]"<<p.second<<endl;
//				}
//			}
//			cout<<endl;
//		}
//	}
//}

void PropagateLookaheads() //从DeterminingLookaheads()求出的初始状态不停循环，传播lookahead直到无新增变化
{
	//把每个item的lookahead set 存成一个大vector 单独判断lookahead变没变，因为运算符重载的原因不好判断item
	vector<set<string> > LA; //初始化为INIT lookahead vector
	vector<set<string> > last_LA;
	for (auto S:C) //initialize LA
		for (auto I:S.KernelItems)
			LA.push_back(I.lookahead);

	do
	{
	 	last_LA = LA;
		for (auto &S:C) //for each state S in LR0 C (已经有初始的lookahead INIT)
		{
			for (auto &I : S.KernelItems) //for each kernel item in kernels of state S
			{
				if (!I.lookahead.empty() && !I.propagation.empty()) //item I 有lookahead, 且存在向后传播 (有propagation)
				{
					for (auto ppg:I.propagation)
					{
						int s_index = ppg.first; //state index
						int i_index = ppg.second; //item index in kernels of state
						for (auto la:I.lookahead)
							C[s_index].KernelItems[i_index].lookahead.insert(la); //I所有lookahead add into 目标item
					}

				}
			}
		}
		//更新LA
		LA.clear(); //清空上轮内容
		for (auto S:C) //添加本轮结束后的内容
			for (auto I:S.KernelItems)
				LA.push_back(I.lookahead);

	}while (last_LA!=LA); //必须是内容不等，不能用size

}

void DebugPrint()
{
	cout<<endl<<endl<<"FINAL RESULT LALR:"<<endl;
	for (auto S:C)
	{
		cout<<S.getIndex()<<endl;
		for (auto I : S.KernelItems )
		{
			cout<<I.toString()<<endl;

		}
	}

}

//we can compute the parsing actions generated by I from I's kernel ALONE.
//直接在ACTION GOTO构建中进行
//只有reduce 的时候才用到lookahead, shift的时候已经有扩展的core了，
//reduce的时候 把所有item 的core 过一遍，找到epsilon单独求一下lookahead就可以

//constructing LALR(1) parsing table
//给产生式编号
map<int, pair<string,string> > Index2G; //<index, left->right>
map<pair<string,string>, int> G2Index; //<left->right, index>
void NumberingProductions() //完成两个map, 实现产生式和序号一一对应
{
	int num=0;
	for (auto p:grammar) //p.first is head
		for (auto body:p.second)
		{
			Index2G[num]=make_pair(p.first,body);
			G2Index[make_pair(p.first,body)] = num;
			num++;
		}
}

void PrintNumberedProductions()
{
	cout<<"Numbered Productions:"<<endl;
	for (auto p:Index2G)
	{
		cout<<p.first<<"  "<<p.second.first<<" -> "<<p.second.second<<endl;
	}
}

//action: Terminals + $ ,marked as T
//goto: Nonterminals marked as NT
//a state s, s.transitions[T] indicates shift to (?) , s.transistions[NT] indicates GOTO to(?)  //entry value
//reduce: if item in s is complete, return production,  entry is reduce by production index, use 2 maps to associate produtcion with index.

map< pair<int,string>, string > ACTION;  // <<state index, T+$>, s2 or r3>
map< pair<int,string>, int > GOTO;  //<state index, NT>, state index>

void constructACTIONandGOTO() //we can compute the parsing actions generated by I from I's kernel ALONE.
{
	NumberingProductions();

	for (auto st:C) //for each state st in the new LALR canonical collection C after lookahead propagation
	{
		/* shift entry */
//		int cur_state_index = st.index;
		for (auto T:terminal)
		{
			if (st.transitions[T] != -1 ) //transition不为空 有shift
			{
				string entry = "s" + std::to_string(st.transitions[T]); //s2 indicates shift to state 2
				pair<int,string> a_key = make_pair(st.index,T);
				if (ACTION.find(a_key)!= ACTION.end()) //如果已经在ACTION表中存在对应entry，说明冲突，不是LALR(1)文法
				{
					cerr<<"Error: [shift] Entry Conflict, not LALR(1) Grammar."<<endl;
                    cerr<<"[in state]"<<st.index<<"  [entry]"<<T<<"  [Already exist]"<<ACTION[a_key]<<endl;
					exit(0);
				}
				else
					ACTION[a_key] = entry; //add to table
			}
		}

		/* GOTO entry */
		for (auto NT:non_terminal)
		{
			if (st.transitions[NT] != -1) //transition不为空，有GOTO项存在
			{
				pair<int,string> g_key = make_pair(st.index, NT);
				if (GOTO.find(g_key)!=GOTO.end()) //之前已经存在，说明出现冲突
				{
					cerr<<"Error: [GOTO] Entry Conflict, not LALR(1) Grammar."<<endl;
					cerr<<"[in state]"<<st.index<<"  [entry]"<<NT<<"  [Already exist]"<<GOTO[g_key]<<endl;
					exit(0);
				}
				else
					GOTO[g_key] = st.transitions[NT];
			}
		}

		/* reduce and accept entry*/
		for (auto it: st.KernelItems) //any reduction will be in the kernel unless epsilon (special case handled by precomputing)
		{
			if (it.isAccept()) // S'->S.
			{
				ACTION[make_pair(st.index,EOI)] = "Acc";  //Accept
				break;
			}
			if (it.isComplete() || it.isEpsilon()) //need reduce
			{
//			    cout<<"<state need reduction>"<<st.index<<" <item>"<<it.toString()<<endl;//debug
				int prod_index = G2Index[make_pair(it.getLeft(), it.getRight()) ]; //规约用的产生式编号
				string entry = "r" + std::to_string(prod_index);

				//规约r 只填在lookahead 搜索符下面  it.getLookahead() is a set
				for (auto la:it.getLookahead())
				{
					pair<int,string> a_key = make_pair(st.index, la);
					if (ACTION.find(a_key)!= ACTION.end()) //如果已经在ACTION表中存在对应entry，说明冲突，不是LALR(1)文法
					{
						cerr<<"Error: [reduce] Entry Conflict, not LALR(1) Grammar."<<endl;
						cerr<<"[in state]"<<st.index<<"  [entry]"<<la<<"  [Already exist]"<<ACTION[a_key]<<endl;
						exit(0);
					}
					else
						ACTION[a_key] = entry; //add to table
				}
			}
		}

		// handle special case： epsilon not in kernels
		for (auto it: st.AllItems)
		{
			if (it.isEpsilon()) //compute lookahead then reduce
			{
//			    cout<<"<state has epsilon>"<<st.index<<" <item>"<<it.toString()<<endl;//debug
				set<string> la4epsilon;
				for (auto ki: st.KernelItems)
				{
					if (ki.getSymbolRight2Dot() == it.getLeft()) //找到这个epsilon item 是由哪个(哪些) kernel item 扩展而来，求出lookahead set
					{
						string str_LA = ki.str2Lookahead();
						set<string> First_set = getStrFirst(str_LA);
						if (First_set.find(EPS)!=First_set.end()) //First set里面有Epsilon，lookahead set添加到扩展项目set里，否则就用不到lookahead
							for (auto la:ki.getLookahead())
								la4epsilon.insert(la);
						//把first set里除了epsilon的symbol 加入lookahead set
						for (auto t:First_set) //for each terminal b in First(Ba)
							if (t!=EPS)
								la4epsilon.insert(t);
					}
				}
				//规约r 只填在lookahead 搜索符下面
				int prod_index = G2Index[make_pair(it.getLeft(), it.getRight()) ]; //规约用的产生式编号
				string entry = "r" + std::to_string(prod_index);
				for (auto lae:la4epsilon)
				{
//				    cout<<"<lookahead>"<<lae<<endl; //debug
					pair<int,string> a_key = make_pair(st.index, lae);
					if (ACTION.find(a_key)!= ACTION.end()) //如果已经在ACTION表中存在对应entry，说明冲突，不是LALR(1)文法
					{
						cerr<<"Error: [Epsilon reduce] Entry Conflict, not LALR(1) Grammar."<<endl;
						cerr<<"[in state]"<<st.index<<"  [entry]"<<lae<<"  [Already exist]"<<ACTION[a_key]<<endl;
						exit(0);
					}
					else
						ACTION[a_key] = entry; //add to table
				}
			}
		}
		// end of this state entry line
	}
}

void PrintParsingTable() //print parsing table
{
	vector<string> A_head(terminal.begin(),terminal.end()); //Table head under Action, Terminals + $
	A_head.push_back(EOI);
	vector<string> G_head; //(non_terminal.begin(),non_terminal.end()); //Table head under Goto, Nonterminals
	for (auto n:non_terminal)
		if (n!=startSymbol)
			G_head.push_back(n);

	cout<<std::left<<setw(10)<<"State"<<setw(A_head.size()*9)<<"Action"<<setw(G_head.size()*9)<<"Goto"<<endl;
	cout<<setw(10)<<" ";
	for (auto x:A_head)
		cout<<setw(9)<<x;
	for (auto x:G_head)
		cout<<setw(9)<<x;
	cout<<endl;

	//C.size() indicates the number of all states.
	for (int i=0;i<C.size();i++)
	{
		cout<<std::left<<setw(10)<<i; //输出状态编号
		for (auto x:A_head)
			if (ACTION.find(make_pair(i,x))!= ACTION.end()) //map中存在entry才输出
				cout<<setw(9)<<ACTION[make_pair(i,x)];
			else
				cout<<setw(9)<<" ";
		for (auto x:G_head)
			if (GOTO.find(make_pair(i,x))!= GOTO.end())  //map中存在entry才输出
				cout<<setw(9)<<GOTO[make_pair(i,x)];
			else
				cout<<setw(9)<<" ";
		cout<<endl;
	}

}

string getStackString(stack<int> stk) //返回分析栈的内容 //反转后
{
	string res; //不能直接反转字符串
	vector<int> tmp;
	while (!stk.empty()) //copy to vector
	{
		tmp.push_back(stk.top());
		stk.pop();
	}
	for (int i=tmp.size()-1;i>=0;i--)
	{
		res+=std::to_string(tmp[i] )+" ";
	}
	return res;
}

string getQueueString(queue<string> q) //返回输入队列的内容
{
	string res;
	while (!q.empty())
	{
		res+=q.front()+" ";
		q.pop();
	}
	return res;
}


void LR_Parsing(string input)
{
//	cout<<std::left<<setw(30)<<"Stack"<<std::right<<setw(30)<<"Input Queue"<<setw(30)<<"Action"<<endl;

	stack<int> stk;  //parsing stack, 存state index, 没存symbol 也许可以再搞一个symbol stack?
	queue<string> q; //input queue

	stk.push(0);	//initial state 0 push onto stack //这里要注意开始状态是不是序号0！

	vector<string> tokens=split_str(input);	//输入串按空格拆成tokens
	for (auto token:tokens)
		q.push(token); //输入串入队
	q.push(EOI); //最后$入队

	while (true)
	{
		//输出当前栈和队列内容
		string stkStr=getStackString(stk);
//		cout<<std::left<<setw(30)<<stkStr;
		string qStr=getQueueString(q);
//		cout<<std::right<<setw(30)<<qStr;
//		cout<<setw(20)<<" ";
        cout<<"[Parsing Stack]\t"<<stkStr<<endl;
        cout<<"[Input Queue]\t"<<qStr<<endl;

		int top = stk.top();
		string front = q.front();

		//然后开始查表
		// input都是terminal组成，所以查表查ACTION map
		pair<int,string> a_key = make_pair(top,front); //make_pair(top,front) is the key of ACTION map
		if (ACTION.find(a_key)!=ACTION.end()) //查表entry存在
		{
			string act = ACTION[a_key];
			if (act[0]=='s') //shift index
			{
				int index = stoi(act.substr(1)); //获取s后面跟的编号，可能不止一位数，所以是string to int
				stk.push(index);	//push index onto the stack
				q.pop(); //front出队，let front be the next input symbol
				cout<<"[Action]\t"<<"Shift"<<endl;
			}
			else if (act[0]=='r')  //reduce index  //reduce left->right
			{
				int index = stoi(act.substr(1)); //获取r后面跟的编号，可能不止一位数，所以是string to int

				//Index2G[index] gets the pair of left->right
				string left = Index2G[index].first;
				string right = Index2G[index].second;

				//output the action: reduce production
				cout<<"[Action]\t"<<"Reduce by "<<left<<" -> "<<right<<endl;

				//pop |right| symbols off the stack
				vector<string> symbols_on_right = split_str(right);
				if (symbols_on_right[0]!="&") //如果为epsilon 等效于不出栈
					for (int i=0;i<symbols_on_right.size();i++)
						stk.pop();

				//then get the top of the stack (e.g. state t)
				int cur_top = stk.top();
				//push GOTO[t,left] onto the stack
				pair<int,string> g_key = make_pair(cur_top, left); //make_pair(cur_top,left) is the key of GOTO map
				//search in GOTO map 查表GOTO确定栈顶新状态
				if (GOTO.find(g_key)!=GOTO.end() ) //查表entry存在
				{
					stk.push( GOTO[g_key] );
				}
				else //查表entry为空，不存在，说明Error
				{
					cerr<<"Syntax Error: Not Found in GOTO"<<endl;  //对应手写是进行下一行stack时报错
					cerr<<"[Error Position]:"<<tokens.size()-q.size()<<endl;
					exit(0);
				}
			}
			else if (act=="Acc") //Accept, parsing is done.
			{
				cout<<"Accept"<<endl;
				cout<<"<Success!> "<<"The input program has no syntax errors."<<endl;
				break;
			}
			else
			{
				cout<<"Error: OtherException"<<endl;
				exit(0);
			}
		}
		else //查表entry为空，不存在，说明Error
		{
			cerr<<"Syntax Error: Not Found in ACTION"<<endl;
			cerr<<"[Error Position]:"<<tokens.size()-q.size()<<endl;
			exit(0);
		}
		cout<<endl;
	}

}


int main()
{
	string grammar_filename = "grammar.txt";
    ifstream fin(grammar_filename);

    if (!fin)
	{
		cerr << "Error: could not open input file'" << grammar_filename << "'" << endl;
    	return 1;
  	}

  	string s;
	while (getline(fin,s))
	{
		cout<<s<<endl;
		gram.push_back(s);
	}

	fin.close();

//	string input;
//	cout<<"Please input a string to parse:"<<endl;
//	getline(cin,input);

//	string input = "identifier := number + number ; identifier := number ; read identifier ; write ( number + number ) * number ; if identifier < number then identifier := number else identifier := number end ; repeat identifier := number until number";

    string input_filename = "tokendone.txt";
    fin.open(input_filename);
    if (!fin)
    {
        cerr << "Error: could not open input file'" << input_filename << "'" << endl;
    	return 1;
    }

    // 使用stringstream将文件内容读入到字符串中
    stringstream buffer;
    buffer << fin.rdbuf();
    string input = buffer.str();
    cout<<"[string to parse]: "<<input<<endl;
    fin.close();

    //input字符串需要将两头空格都去掉，中间只能以一个空格作为delimiter，(Scanner 已处理)


	cout<<"reading done."<<endl;

	split_pro();
	get_symbols();
	augmentedGrammar();
	cout<<"processing done."<<endl;

	cout<<"N:  ";
	for (auto i:non_terminal)
		cout<<i<<"  ";
	cout<<non_terminal.size();
	cout<<endl<<"T:  ";
	for (auto i:terminal)
		cout<<i<<"  ";
	cout<<terminal.size();
	cout<<endl<<endl<<"Augmented Grammar:"<<endl;
	for (auto p:grammar)
		for (auto i:p.second)
			cout<<p.first<<" -> "<<i<<endl;

	cout<<endl;

	getCanonicalCollection();
	DeterminingLookaheads();
//	DebugPrint();
	PropagateLookaheads();
	DebugPrint();
	cout<<endl;

	constructACTIONandGOTO();
	cout<<endl;
	PrintNumberedProductions();
	cout<<endl;

	PrintParsingTable();
	cout<<endl;

	LR_Parsing(input);



//test case: LR0 acc
//E' -> E
//E -> ( L )
//E -> a
//L -> L , E
//L -> E


//test case: SLR(1)
//E' -> E
//E -> E + T
//E -> T
//T -> T * F | F
//F -> ( E ) | id

//test case: LR(1)
//S -> C C
//C -> c C | d

//test case: LR(1)
//A -> ( A ) | a

//test case: LALR(1) textbook
//S -> L = R | R
//L -> * R | id
//R -> L

	return 0;
}

