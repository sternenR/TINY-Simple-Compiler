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
//�涨����ÿ��token�м䶼Ҫ�ӿո�
//A -> a A | B b | d
//��->,| ֮��Ҳ�ӿո�
//Data Structure ����LL1

set<string> non_terminal;
set<string> terminal;

// data structure of grammar
vector<string> gram; //grammar with raw productions

//grammar after preprocess
map<string, vector<string> > grammar; //ǰ����Nt,�����ж��choice string (if it has)

string startSymbol; //��ʼ���ţ���Ϊ�����һ������ʽ����� �ع��ķ��󱻸ı�

const string EOI="$";
const string NIG="#"; //a symbol not in grammar, used in determining lookaheads (propagated or spontaneous)
const string EPS="&"; //epsilon ��&�����

void split_pro() //��ֲ���ʽ
{
	for (auto p:gram)
	{
		string lhs,rhs;

		for (int i=0;i<p.size()-1;i++)
		{
			if (p[i]=='-' && p[i+1]=='>')
			{
				lhs=p.substr(0,i-1);  //update here cuz the white space before ->
				rhs=p.substr(i+3); //para_2nd default=npos ���ӵڶ�������������stringĩβ i.e. substr(i+2, string::npos) //update here cuz the white space after ->
				break;
			}
		}
		//rhs ��ͷû�пո�
		int start=0,length=0; //indicate positions to split production contains | ÿ��substr��param �зֵ�λ�úͳ���
		for (int i=1;i<rhs.size()-1;i++) //split | on rhs
		{
			if (rhs[i-1]==' ' && rhs[i]=='|' && rhs[i+1]==' ') //delimiter changed from '|' to 'space|space'
			{
				length=i-start-1;
				grammar[lhs].push_back(rhs.substr(start,length));
				start=i+2; //���¿�ʼλ��
			}
		}
		grammar[lhs].push_back(rhs.substr(start)); //ѭ�����������ʣһ����start���λ�ý�ȡ��ĩβ���������ʽ��û����|��start����0����ͷ��ȡ��β��string rhs����
	}
}

vector<string> split_str(string str) //��string s ����ȡ�ÿո�����ÿ������
{ //��split_pro()��ֲ���ʽ ������split_str() ����ȡNT�� ��Ϊsplit_strĬ��string��û��| ��
	vector<string> tokens;
	string s=str; //copy string����
	string delimiter=" ";
	int pos=0;
	while ((pos=s.find(delimiter))!=string::npos)
	{
		string token=s.substr(0,pos);
		tokens.push_back(token);
		s.erase(0,pos+delimiter.length()); //��ǰ����ȡ���Ĳ���ɾ��
	}
	tokens.push_back(s);

	return tokens;
}

string merge_str(vector<string> tokens) //��һ��vector string����Ͽո񣬺ϳ�һ��������string ����ͷû�ӿո�
{
	string res=tokens[0];
	for (int i=1;i<tokens.size();i++)
		res+=" "+tokens[i];
	return res;
}

void get_symbols() //��ȡN��T����
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
	{ //ע������split_pro���grammar����������ԭ�ַ���gram
		for (auto p:line.second)
		{
			//Ҫ��split_pro ������split_str ��Ϊsplit_strĬ��string��û��| ��
			vector<string> tokens = split_str(p);
			for (auto token:tokens)
			{
//				cout<<"@"<<token<<"@"<<endl;//debug
				if (token!="&" && token!="" && token.size()>0 && non_terminal.find(token) == non_terminal.end()) //��Ϊ��(&) �� ����non_terminal set��
					terminal.insert(token); //����terminal set
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

set<string> getStrFirst(string s) //get first set of a specific string //ע��������Ҫ���⴦�� $ �� �� s is empty���������LL�ﲻһ��
{
	set<string> strFirst;

	if (s.empty() || s=="")
	{
		strFirst.insert(EPS);
		return strFirst;
	}

	if(s==EOI)  //First($) = {$} //lookahead�ϲ���s�������$
	{
		strFirst.insert(EOI);
		return strFirst; //����
	}

	vector<string> tokens=split_str(s);
	int epsilon_num=0; //ͳ���ܱ�ɿյ�Nonterminal����

	for (auto token:tokens)
	{
		if (token=="&" && tokens.size()==1) //���Ҫ����ַ�����ֻ�Ц�
		{
			strFirst.insert("&");
			return strFirst; //����� ֱ�ӽ���
		}

		int flag=0; //���費�������ٿ��ˣ���ǰtoken��terminal,��ǰtoken��nonterminal�����ܱ�ɿգ�
		if (isTerminal(token))
		{
			strFirst.insert(token);
			break;
		}
		else if (isNonTerminal(token))
		{
			//First[token] ��token ��first set
			for (auto x:First[token])
				if (x!="&") //ȥ���ż��뵱ǰstr��first set
					strFirst.insert(x);
				else //x==&
				{
					flag=1;
					epsilon_num++;
				}

		}

		if(!flag) //��ǰ��non-terminal���ܱ�ɦţ��Ͳ��������ˡ�
			break;
	}
	if (epsilon_num==tokens.size()) //��ͷ��βÿһ�����ܱ�ɿգ�˵��������Ҳ���Ƴ���
		strFirst.insert("&");
	return strFirst;
}

void getFirst() // get first sets of all non_terminals
{
	map<string, set<string> > oldFirst; //������һ�ֱ�����First
	while (true)
	{
		oldFirst=First;  //��һ�ֱ�����First ����oldFirst ��һ�ָ���First ������Ա�

		for (auto it:grammar) //�����в���ʽ����һ��
		{
			string curNt = it.first;
			for (auto str:it.second)
			{
				set<string> addFirst = getStrFirst(str);
				First[curNt].insert(addFirst.begin(),addFirst.end());
			}
		}

		if (oldFirst==First) //ֱ������Firstû�б仯��ʱ�����
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
	for (auto it:grammar ) //��ÿ������ʽ
	{
		int flag=0; //�ȼ��費��Ҫ����ߵ�follow���Ͻ�ȥ
		for (auto str:it.second) //str��ÿ������ʽ�ұߵ��ַ���
		{
			int pos=0;
			while ((pos = str.find(N))!=string::npos) //int pos = str.find(N); //���ұ���N��λ��  //	if(pos!=-1) //����ҵ�N
			{ //N������Ӵ���First ���ϼ�ȥ�� ����Follow(N)��

				if (pos+N.size()>=str.size()) //���������ҵ�N������û�и��κ��ַ���(��ʵ���൱�ں�����Ĵ���first set���п�)��ҲҪ�Ѳ���ʽ��߷��ŵ�Follow ���ϼӵ�N��Follow������
				{
					flag=1;
					break; //Ҳ�����������ˣ�str�Ѿ���ͷ��
				}
				string afterN = str.substr(pos+N.size()+1); //�ո�
				set<string> First2add=getStrFirst(afterN); //N������Ӵ���First ����
				for (auto x:First2add)
				{
					if (x!="&") //ȥ����
						FollowN.insert(x);
					else // if (x=="&") N������Ӵ���First �����п�
						flag=1; //��Ҫ��it.first Ҳ�����������ʽ��ߵķ��� ��Follow ���ϼӵ�N��Follow������
				}
			 	str=afterN; //����str ǰ���ҹ��Ĳ���ȥ����������û�ҵĲ����ټ�������û��N
			}
		}
		if(flag) //����ʽ��߷��ŵ�Follow ���ϼӵ�N��Follow������
			FollowN.insert(Follow[it.first].begin(), Follow[it.first].end());
	}
	return FollowN;
}

void getFollow()
{
	Follow[startSymbol].insert("$"); //��ʼ���ź���һ�����ս��$
	map<string, set<string> > oldFollow; //������һ�ֱ�����Follow
	while (true)
	{
		oldFollow=Follow;

		for (auto it:grammar ) //��ÿ������ʽit
		{
			for (auto str:it.second) //str��ÿ������ʽ�ұߵ��ַ���
			{
				vector<string> tokens=split_str(str); //ÿ������ʽ�ұ߲��һ��һ������
				for (auto token:tokens) //��ÿһ������
				{
					if (isNonTerminal(token)) //����Ƿ��ս������һ������follow���� �ټӵ���Ӧ��follow������
					{
						set<string> addFollow = getFollow_ofN(token);
						Follow[token].insert(addFollow.begin(),addFollow.end());
					}
				}
			}
		}
		//ɨ��һ��
		if (oldFollow==Follow) //ֱ������Followû�б仯ʱ����
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

void augmentedGrammar() //�ع��ķ�
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
//		string lookahead; //��������û�кϲ���������������Ƕ��item
		set<string> lookahead; //�ϲ�������
//to deal with lookaheads propagations
//1. propagation of lookaheads
//2. spontaneously generated lookaheads for kernels in each state
		//spontaneous ֱ�Ӵ浽item��lookahead vector�У���ΪINIT
		vector<pair<int,int> > propagation; //propagate to which state and which item <state index,item index> //obtain item index via vector iterator-begin()


	public:
		item(string left="", string right="", int dotPos=0); //, string lookahead="" ); //�������Ĺ��캯�� ��������Ĭ��ֵ lookahead="#" is used to compute propagation of lookaheads
		string toString(); //return production string with dot
		string toOriginalString(); //return production string without dot

		string getLeft();
		string getRight();
		int getDotPos();
		set<string> getLookahead();

		void increaseDotPos(); //dot ����һλ
		string getSymbolRight2Dot();  // return the symbol immediately to the right of the dot in current produtcion
		bool isComplete(); //if this item is complete item (i.e. dot is on the right end)
		bool isAccept(); //especially, if this item is S'->S.
		bool isEpsilon(); //�ж�item ����ʽ�Ҳ��Ƿ�Ϊ��

		string str2Lookahead(); //�����ǰitem,dot������Nonterminal B�����ص�ǰitem��B����������ַ���(������lookahead) ĩβ�޿ո�

		//��������� overload  ������lookahead
		bool operator==(const item& right) const
		{
      		return this->left == right.left && this->right==right.right && this->dotPos== right.dotPos; // && this->lookahead == right.lookahead;
  		}
	  	bool operator!=(const item& right) const
		{
	      	return this->left != right.left && this->right!=right.right && this->dotPos != right.dotPos; // && this->lookahead != right.lookahead;
	  	}

	  	//���<�������������ᵼ��״̬������ //�ֳɶ��д
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
 		friend bool isNextItem(item i1, item i2); //�ж�i2�Ƿ�Ϊi1��next������DotPos������һ��������lookahead
 		friend void PropagateLookaheads();
		friend void DebugPrint();

};

//// LR(1) item, production with a dot. and a lookahead component //e.g.: S -> a .B , a
//class item1:protected item  //inherited from class item
//{
//	private:
//		string lookahead; //��������û�кϲ���������������Ƕ��item
//
//	public:
//		item1(string left="", string right="", int dotPos=0, string lookahead=""):item(left,right,dotPos) //���캯������ʼ���б���û��๹�캯��
//		{
//			this->lookahead=lookahead;
//		}
//
//		string getLookahead();
//		void setLookahead(string x);
//
//		string str2Lookahead(); //�����ǰitem,dot������Nonterminal B�����ص�ǰitem��B����������ַ���(����lookahead)
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
			res+=tokens[i]; //ĩβ���ӿո�
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
		return ""; //������󣬺���û�з����ˣ��������Թ�Լ�����ؿ� //�����ж��Ƿ��Լ��װ����һ��������
	if (dotPos>tokens.size())
	{
		cerr<<"Error: Invalid Dot Position! \ngetSymbolRight2Dot() Failed."<<endl;
		exit(0);
	}
	return tokens[dotPos];
}

string item::str2Lookahead() //�����ǰitem,dot������Nonterminal B�����ص�ǰitem��B����������ַ���(������lookahead) ��󲻴��ո� //���B����û���ַ��ˣ�res����empty
{
	string res;
	string nextSymbol=getSymbolRight2Dot();
	vector<string> tokens = split_str(right);
	if (isNonTerminal(nextSymbol) )
	{
		for (int i=dotPos+1; i<tokens.size()-1; i++)
			res+=tokens[i]+" ";
		if (dotPos+1<tokens.size())
			res+=tokens[tokens.size()-1]; //���һ�����⴦��ĩβ���ӿո�
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

bool isNextItem(item i1, item i2) //�ж�i2�Ƿ�Ϊi1��next������DotPos������һ��������lookahead
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
		for (int it=0;it<J.size();it++) //������ѭ�����ܵ����ڴ�ʧЧ����Ϊ�±�ѭ�� ��������it��Ϊ J[it] //for (item it:J)  //for each item in J
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
	getFirst(); //�����First���ϱ���

	vector<item> J=I;
	vector<item> lastJ; //store J on last one round to check if there are new items added into J

////lookahead�Ѻϲ�������added��������LR0
	//����addedҪ�ж�NT��lookahead string ��û�ӹ�����
	map< pair<string,string >, bool> added; //<NT,str_LA> ,if it is added to the set>
	//����initialize added��û�ӹ����ǿգ�find�Ҳ������ӹ�����true

	//���added������Ļ��ͻᵼ��closure()����

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
						if (exist!=J.end()) //���newItem�Ѿ���J����ڣ�������lookahead vector���� (*exist).lookahead
						{
							set<string> First_set = getStrFirst(str_LA);
							if (First_set.find(EPS)!=First_set.end()) //First set������Epsilon��lookahead set��ӵ���չ��Ŀset�������ò���lookahead
								for (auto la:it.lookahead)
									(*exist).lookahead.insert(la);
							//��first set�����epsilon��symbol ����lookahead vector
							for (auto t:First_set) //for each terminal b in First(Ba)
							{
								if (t!=EPS)
									(*exist).lookahead.insert(t);
							}
						}
						else //newItem not in J
						{
							set<string> First_set = getStrFirst(str_LA);
							if (First_set.find(EPS)!=First_set.end()) //First set������Epsilon��lookahead set��ӵ���չ��Ŀset�������ò���lookahead
								for (auto la:it.lookahead)
									newItem.lookahead.insert(la);
							//��first set�����epsilon��symbol ����lookahead vector
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
		static int num; //����
		int index;//״̬���
		vector<item> KernelItems; //��Ҫ�ǿ� kernel items
		vector<item> AllItems;
		mutable map<string, int> transitions; //goto sets  //this state via <string> X to <int>index state  //maybe set to public??
		//�൱����transitions ������������goto()�Ĺ��� ��getcanonical������������� ����д��һ�������е�����
	public:
		state();
		state(vector<item> KernelItems, vector<item> AllItems); //���ǰ�AllItemsȥ�������˹�Լepsilon��ʱ���ò���
//		Goto();
		int getIndex();

		//overload operator for set.insert
		bool operator==(const state& right) const
		{
      		return this->KernelItems == right.KernelItems; //��kernelItems,����index
  		}
	  	bool operator!=(const state& right) const
		{
	      	return this->KernelItems != right.KernelItems;
	  	}
	  	bool operator<(const state& right) const  //better ways?
		{
	      	return this->KernelItems < right.KernelItems;
	  	}

		friend void getCanonicalCollection(); //��Ԫ����
		friend void DeterminingLookaheads(); //���each item��Ӧ�� propagate from ? to ? ��ϵ  �� spontaneous set ����lookahead set��
		friend void constructACTIONandGOTO();
		friend void PropagateLookaheads();
		friend void DebugPrint();
};

int state::num=0; //��̬��Ա���������ʼ�� �������ⲿ��ʼ��

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
			if (find(nextI.begin(),nextI.end(),nextItem)==nextI.end()) //ȥ��
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
//setԪ���޷��޸ģ��޷�����lookahead computation loop �޸�state item��lookahead,����vector+ȥ���ж�
vector<state> C; //global //C��indexӦ�ú�state��index�Ƕ�Ӧ���ϵ�
//compute the canonical collection of sets of LR(0) items
void getCanonicalCollection()  //maybe return set<state>
{
	item i0(startSymbol, grammar[startSymbol][0], 0);
	//spontaneous ֱ�Ӵ浽item��lookahead vector�У���ΪINIT
	i0.lookahead.insert(EOI); //��������I0's INIT spontaneous lookahead is EOI($)

	vector<item> I0 = { i0 }; //I0 is originally set to the augmented grammar S' -> .S
	//kernel items: I0
	//all items: closure(I0)

	state S0 = state(I0, closure0(I0));  //���ù��캯��ʱnum�Զ�++
//	set<state>
	C = {S0};
//	vector<state> lastC;
	int lastCsize;

	do
	{
		lastCsize=C.size();

		//�õ����������ڴ�ʧЧ�������±����vector //for (auto &S:C) //for each set of items (states) in C //Ҫ���� ��Ϊ��Ҫ�޸�S�ĳ�Ա����transitions��mutable������뱨��
		for (int j=0;j<C.size();j++) //��C[j]�滻����S
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

				if (!next_kItems.empty()) //not empty //ֻҪ���վ�˵��������һ��transitionָ��ȥ��Ҫ��������߲���
				{
					state nextS(next_kItems, closure0(next_kItems)); //num++ automatically
					auto iter= find(C.begin(),C.end(),nextS);
					if (iter!=C.end()) //this new state is already in set C
					{
						state::num--; //��ż���ȥ
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


//α����
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
				if (B.isComplete() || B.isEpsilon())  //B is complete item or epsilon, dot�����û����һ��״̬
					continue;

				string X = B.getSymbolRight2Dot();
				int s_index = S.transitions[X]; //��һ��״̬���
				int i_index; //�� B->rX.g in GOTO(I,X) ��next state��kernelItems vector�е�index

//				cout<<"[state]"<<S.index<<" --- "<<X<<"--- "<<s_index<<endl; //debug

				for(auto la: B.getLookahead() )
				{
					if (la!= NIG) //a is not #, conclude that lookahead is generated spontaneously for item B->rX.g in GOTO(I,X)
					{ //�ҵ���һ��item���ĸ�state��ʲôλ�ã���spontaneous��¼
						for (auto it = C[s_index].KernelItems.begin(); it !=C[s_index].KernelItems.end(); ++it )//C[s_index]ͨ��vector�±������һ��state
						{
							if (isNextItem(B,*it)) //find item B->rX.g in GOTO(I,X)
								(*it).lookahead.insert(la); //spontaneous �ı�ԭ�е�lookahead ��ΪINIT
						}
					}
					else if (la==NIG) // a is #, conclude that lookaheads propagate from A->a.C in I to B->rX.g in GOTO(I,X)
					{ //��itm��propagation ����¼����¼�ᴫ������Щstate����Щitem
						for (auto it = C[s_index].KernelItems.begin(); it !=C[s_index].KernelItems.end(); ++it )//C[s_index]ͨ��vector�±������һ��state
						{
							if (isNextItem(B,*it)) //find item B->rX.g in GOTO(I,X)
								i_index = distance( C[s_index].KernelItems.begin(), it); // �����������vector.begin()֮��ľ���
						}

						itm.propagation.push_back(make_pair(s_index,i_index)); //Ҫ�ı�itm.propagation�洢��ֵ
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

void PropagateLookaheads() //��DeterminingLookaheads()����ĳ�ʼ״̬��ͣѭ��������lookaheadֱ���������仯
{
	//��ÿ��item��lookahead set ���һ����vector �����ж�lookahead��û�䣬��Ϊ��������ص�ԭ�򲻺��ж�item
	vector<set<string> > LA; //��ʼ��ΪINIT lookahead vector
	vector<set<string> > last_LA;
	for (auto S:C) //initialize LA
		for (auto I:S.KernelItems)
			LA.push_back(I.lookahead);

	do
	{
	 	last_LA = LA;
		for (auto &S:C) //for each state S in LR0 C (�Ѿ��г�ʼ��lookahead INIT)
		{
			for (auto &I : S.KernelItems) //for each kernel item in kernels of state S
			{
				if (!I.lookahead.empty() && !I.propagation.empty()) //item I ��lookahead, �Ҵ�����󴫲� (��propagation)
				{
					for (auto ppg:I.propagation)
					{
						int s_index = ppg.first; //state index
						int i_index = ppg.second; //item index in kernels of state
						for (auto la:I.lookahead)
							C[s_index].KernelItems[i_index].lookahead.insert(la); //I����lookahead add into Ŀ��item
					}

				}
			}
		}
		//����LA
		LA.clear(); //�����������
		for (auto S:C) //��ӱ��ֽ����������
			for (auto I:S.KernelItems)
				LA.push_back(I.lookahead);

	}while (last_LA!=LA); //���������ݲ��ȣ�������size

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
//ֱ����ACTION GOTO�����н���
//ֻ��reduce ��ʱ����õ�lookahead, shift��ʱ���Ѿ�����չ��core�ˣ�
//reduce��ʱ�� ������item ��core ��һ�飬�ҵ�epsilon������һ��lookahead�Ϳ���

//constructing LALR(1) parsing table
//������ʽ���
map<int, pair<string,string> > Index2G; //<index, left->right>
map<pair<string,string>, int> G2Index; //<left->right, index>
void NumberingProductions() //�������map, ʵ�ֲ���ʽ�����һһ��Ӧ
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
			if (st.transitions[T] != -1 ) //transition��Ϊ�� ��shift
			{
				string entry = "s" + std::to_string(st.transitions[T]); //s2 indicates shift to state 2
				pair<int,string> a_key = make_pair(st.index,T);
				if (ACTION.find(a_key)!= ACTION.end()) //����Ѿ���ACTION���д��ڶ�Ӧentry��˵����ͻ������LALR(1)�ķ�
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
			if (st.transitions[NT] != -1) //transition��Ϊ�գ���GOTO�����
			{
				pair<int,string> g_key = make_pair(st.index, NT);
				if (GOTO.find(g_key)!=GOTO.end()) //֮ǰ�Ѿ����ڣ�˵�����ֳ�ͻ
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
				int prod_index = G2Index[make_pair(it.getLeft(), it.getRight()) ]; //��Լ�õĲ���ʽ���
				string entry = "r" + std::to_string(prod_index);

				//��Լr ֻ����lookahead ����������  it.getLookahead() is a set
				for (auto la:it.getLookahead())
				{
					pair<int,string> a_key = make_pair(st.index, la);
					if (ACTION.find(a_key)!= ACTION.end()) //����Ѿ���ACTION���д��ڶ�Ӧentry��˵����ͻ������LALR(1)�ķ�
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

		// handle special case�� epsilon not in kernels
		for (auto it: st.AllItems)
		{
			if (it.isEpsilon()) //compute lookahead then reduce
			{
//			    cout<<"<state has epsilon>"<<st.index<<" <item>"<<it.toString()<<endl;//debug
				set<string> la4epsilon;
				for (auto ki: st.KernelItems)
				{
					if (ki.getSymbolRight2Dot() == it.getLeft()) //�ҵ����epsilon item �����ĸ�(��Щ) kernel item ��չ���������lookahead set
					{
						string str_LA = ki.str2Lookahead();
						set<string> First_set = getStrFirst(str_LA);
						if (First_set.find(EPS)!=First_set.end()) //First set������Epsilon��lookahead set��ӵ���չ��Ŀset�������ò���lookahead
							for (auto la:ki.getLookahead())
								la4epsilon.insert(la);
						//��first set�����epsilon��symbol ����lookahead set
						for (auto t:First_set) //for each terminal b in First(Ba)
							if (t!=EPS)
								la4epsilon.insert(t);
					}
				}
				//��Լr ֻ����lookahead ����������
				int prod_index = G2Index[make_pair(it.getLeft(), it.getRight()) ]; //��Լ�õĲ���ʽ���
				string entry = "r" + std::to_string(prod_index);
				for (auto lae:la4epsilon)
				{
//				    cout<<"<lookahead>"<<lae<<endl; //debug
					pair<int,string> a_key = make_pair(st.index, lae);
					if (ACTION.find(a_key)!= ACTION.end()) //����Ѿ���ACTION���д��ڶ�Ӧentry��˵����ͻ������LALR(1)�ķ�
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
		cout<<std::left<<setw(10)<<i; //���״̬���
		for (auto x:A_head)
			if (ACTION.find(make_pair(i,x))!= ACTION.end()) //map�д���entry�����
				cout<<setw(9)<<ACTION[make_pair(i,x)];
			else
				cout<<setw(9)<<" ";
		for (auto x:G_head)
			if (GOTO.find(make_pair(i,x))!= GOTO.end())  //map�д���entry�����
				cout<<setw(9)<<GOTO[make_pair(i,x)];
			else
				cout<<setw(9)<<" ";
		cout<<endl;
	}

}

string getStackString(stack<int> stk) //���ط���ջ������ //��ת��
{
	string res; //����ֱ�ӷ�ת�ַ���
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

string getQueueString(queue<string> q) //����������е�����
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

	stack<int> stk;  //parsing stack, ��state index, û��symbol Ҳ������ٸ�һ��symbol stack?
	queue<string> q; //input queue

	stk.push(0);	//initial state 0 push onto stack //����Ҫע�⿪ʼ״̬�ǲ������0��

	vector<string> tokens=split_str(input);	//���봮���ո���tokens
	for (auto token:tokens)
		q.push(token); //���봮���
	q.push(EOI); //���$���

	while (true)
	{
		//�����ǰջ�Ͷ�������
		string stkStr=getStackString(stk);
//		cout<<std::left<<setw(30)<<stkStr;
		string qStr=getQueueString(q);
//		cout<<std::right<<setw(30)<<qStr;
//		cout<<setw(20)<<" ";
        cout<<"[Parsing Stack]\t"<<stkStr<<endl;
        cout<<"[Input Queue]\t"<<qStr<<endl;

		int top = stk.top();
		string front = q.front();

		//Ȼ��ʼ���
		// input����terminal��ɣ����Բ���ACTION map
		pair<int,string> a_key = make_pair(top,front); //make_pair(top,front) is the key of ACTION map
		if (ACTION.find(a_key)!=ACTION.end()) //���entry����
		{
			string act = ACTION[a_key];
			if (act[0]=='s') //shift index
			{
				int index = stoi(act.substr(1)); //��ȡs������ı�ţ����ܲ�ֹһλ����������string to int
				stk.push(index);	//push index onto the stack
				q.pop(); //front���ӣ�let front be the next input symbol
				cout<<"[Action]\t"<<"Shift"<<endl;
			}
			else if (act[0]=='r')  //reduce index  //reduce left->right
			{
				int index = stoi(act.substr(1)); //��ȡr������ı�ţ����ܲ�ֹһλ����������string to int

				//Index2G[index] gets the pair of left->right
				string left = Index2G[index].first;
				string right = Index2G[index].second;

				//output the action: reduce production
				cout<<"[Action]\t"<<"Reduce by "<<left<<" -> "<<right<<endl;

				//pop |right| symbols off the stack
				vector<string> symbols_on_right = split_str(right);
				if (symbols_on_right[0]!="&") //���Ϊepsilon ��Ч�ڲ���ջ
					for (int i=0;i<symbols_on_right.size();i++)
						stk.pop();

				//then get the top of the stack (e.g. state t)
				int cur_top = stk.top();
				//push GOTO[t,left] onto the stack
				pair<int,string> g_key = make_pair(cur_top, left); //make_pair(cur_top,left) is the key of GOTO map
				//search in GOTO map ���GOTOȷ��ջ����״̬
				if (GOTO.find(g_key)!=GOTO.end() ) //���entry����
				{
					stk.push( GOTO[g_key] );
				}
				else //���entryΪ�գ������ڣ�˵��Error
				{
					cerr<<"Syntax Error: Not Found in GOTO"<<endl;  //��Ӧ��д�ǽ�����һ��stackʱ����
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
		else //���entryΪ�գ������ڣ�˵��Error
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

    // ʹ��stringstream���ļ����ݶ��뵽�ַ�����
    stringstream buffer;
    buffer << fin.rdbuf();
    string input = buffer.str();
    cout<<"[string to parse]: "<<input<<endl;
    fin.close();

    //input�ַ�����Ҫ����ͷ�ո�ȥ�����м�ֻ����һ���ո���Ϊdelimiter��(Scanner �Ѵ���)


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

