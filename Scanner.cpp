#include <bits/stdc++.h>
#define _KEY_WORDEND "waiting for your expanding"
using namespace std;
typedef struct //词的结构，二元组形式（单词种别，单词自身的值）
{
	int typenum; //单词种别
	char * word;
}WORD;
char input[255];
char token[255] = "";
int p_input; //指针
int p_token;
char ch;
char * rwtab[] = { "if","then","else","end","repeat","until","read","write","int","main","float","double","return",_KEY_WORDEND };
/*
char * rwtab[] = { "if","then","else","end","repeat","until","read","write","while","do","int","main",
                        "float","double","return","cout",_KEY_WORDEND };
*/

WORD * scanner();//扫描

//从输入缓冲区读取一个字符到ch中
char Obtain2ch()
{
	ch = input[p_input];
	p_input++;
	return ch;
}

//去掉空白符号
void deleteblank()
{
	while (ch == ' ' || ch == 10)
	{
		ch = input[p_input];
		p_input++;
	}
}

// 拼接单词
void split()
{
    if (p_token >= 10000) {
        // 单词缓冲区已满，报错
        printf("Error: token buffer overflow\n");
        exit(1);
    }

    token[p_token] = ch;
    p_token++;
    token[p_token] = '\0';
}

//判断是否字母
int letter()
{
	if (ch >= 'a'&&ch <= 'z' || ch >= 'A'&&ch <= 'Z')
		return 1;
	else
		return 0;
}

//判断是否数字
int digit()
{
	if (ch >= '0'&&ch <= '9')
    {
        return 1;
    }
	else
    {
        return 0;
    }
}

//检索关键字表格
int reserve()
{
	int i = 0;
	while(strcmp(rwtab[i], _KEY_WORDEND))
	{
		if (!strcmp(rwtab[i], token))
        {
            return i + 1;
        }
		i++;
	}
	return 10;//如果不是关键字，则返回种别码10
}

//回退一个字符
void retract()
{
	p_input--;
}

int main()
{
	int over = 1;
	WORD* oneword = new WORD;

	//实现从文件读取代码段
	cout << "read something from input.txt" << endl;

	FILE *fp;

	fp = fopen("token.txt", "w"); // 打开文件
	fputs("", fp); // 写入数据

	if((fp=freopen("input.txt","r",stdin))==NULL)
        {
                printf("Not found file!\n");
                return 0;
        }
        else
        {
                while ((scanf("%[^#]s", &input)) != EOF)
                {
                        p_input = 0;
                        printf("your words:\n%s\n", input);
                        while (over < 1000 && over != -1)
                        {
                                oneword = scanner();
                                if (oneword->typenum < 1000)
                                {
                                        if(oneword->typenum != 999){
                                                cout << "[  "<< oneword->typenum <<"\t"<< oneword->word <<"  ]"<< endl;
                                                if(oneword->typenum==10)
                                                {
                                                    oneword->word="identifier";
                                                    cout << "[  "<< oneword->typenum <<"\t"<< oneword->word <<"  ]"<< endl;
                                                }
                                                if(oneword->typenum==20)
                                                {
                                                    oneword->word="number";
                                                    cout << "[  "<< oneword->typenum <<"\t"<< oneword->word <<"  ]"<< endl;
                                                }
                                                FILE* fp = fopen("token.txt", "a+"); // 打开文件
                                                if (fp == NULL) {
                                                    printf("无法打开文件\n");
                                                    return 1;
                                                }
                                                fputs(oneword->word, fp); // 写入数据
                                                fputs(" ", fp); // 写入数据
                                                fclose(fp); // 关闭文件
                                                printf("数据已写入文件\n");
                                                                                    }

                                }
                                over = oneword->typenum;
                        }
                        scanf("%[^#]s", input);
                }
        }
    char tokendone[100000];
    FILE *p=fopen("token.txt","r");
    fscanf(p,"%[^#]",tokendone);//#代表待处理文件中不存的字符
    fclose(p);
    int j=0;
    for(int i=0;i<strlen(tokendone);i++){
        if(i==strlen(tokendone)-1) continue;//删除空格和回车
        tokendone[j++]=tokendone[i];
    }
    tokendone[j]=0;//0即 NULL
    p=fopen("tokendone.txt","w");//新建一个in2.txt文件
    fprintf(p,"%s",tokendone);//处理后的文件放在in2.txt中
    fclose(p);
    /*
    char filename[] = "token.txt";
    char buffer[100];
    int file_size, i;

    // 打开文件
    fp = fopen(filename, "r+");
    if (fp == NULL) {
        printf("无法打开文件 %s\n", filename);
        exit(1);
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);

    // 将文件指针移动到倒数第二个字符
    fseek(fp, -2, SEEK_END);

    // 读取最后一个字符
    fgets(buffer, 2, fp);

    // 如果最后一个字符是换行符，则将文件指针再次向前移动一位
    if (strcmp(buffer, "\n") == 0) {
        fseek(fp, -1, SEEK_END);
        file_size--;
    }

    // 将文件指针移动到倒数第二个字符
    fseek(fp, -2, SEEK_END);

    // 将最后一个字符替换为空字符
    fputc('\0', fp);

    // 关闭文件
    fclose(fp);

    printf("成功删除文件 %s 的最后一个字符\n", filename);
    */
    return 0;
}

//词法扫描程序
WORD * scanner()
{
	WORD * myword = new WORD;
	myword->typenum = 10;  //初始值
	myword->word = "";
	p_token = 0;   //单词缓冲区指针
	Obtain2ch();
	deleteblank();//去掉空白

    if (letter())
    {
        // 如果读取到的字符是字母，进入循环
        while (letter() || digit())
        {
            split(); // 连接字符
            Obtain2ch(); // 继续读取下一个字符
        }
        retract(); // 回退一个字符，因为最后读取的字符不是字母或数字
        myword->typenum = reserve(); // 判断单词是否为关键字，返回相应的种别码
        myword->word = token; // 将单词存入结构体中
        return myword; // 返回结构体指针
    }
	else if (digit())  //判断读取到的单词首字符是数字
	{
		while (digit()) //所有数字连接起来
		{
			split();
			Obtain2ch();
		}
		retract();
		//数字单词种别码统一为20，单词自身的值为数字本身
		myword->typenum = 20;
		myword->word = token;
		return(myword);
	}
	else switch (ch)
	{
	case '=':
		Obtain2ch();//首字符为=,再读取下一个字符判断
		if (ch == '=')
		{
			myword->typenum = 39;
			myword->word = "==";
			return(myword);
		}
		retract();//读取到的下个字符不是=，则要回退，直接输出=
		myword->typenum = 21;
		myword->word = "=";
		return(myword);
		break;
	case '+':
		myword->typenum = 22;
		myword->word = "+";
		return(myword);
		break;
	case '-':
		myword->typenum = 23;
		myword->word = "-";
		return(myword);
		break;
        case '/'://读取到该符号之后，要判断下一个字符是什么符号，判断是否为注释
                Obtain2ch();//首字符为/,再读取下一个字符判断
		if (ch == '*') // 说明读取到的是注释
		{
		        Obtain2ch();

			while(ch != '*')
                        {
                                Obtain2ch();//注释没结束之前一直读取注释，但不输出
                                if(ch == '*')
                                {
                                        Obtain2ch();
                                        if(ch == '/')//注释结束
                                        {
                                                myword->typenum = 999;
                                                myword->word = "注释";
                                                return (myword);
                                                break;
                                        }
                                }

                        }

		}
                else
                {
                        retract();//读取到的下个字符不是*，即不是注释，则要回退，直接输出/

                        myword->typenum = 25;
                        myword->word = "/";
                        return (myword);
                        break;
                }
        case '*':
		myword->typenum = 24;
		myword->word = "*";
		return(myword);
		break;
	case '(':
		myword->typenum = 26;
		myword->word = "(";
		return(myword);
		break;
	case ')':
		myword->typenum = 27;
		myword->word = ")";
		return(myword);
		break;
	case '[':
		myword->typenum = 28;
		myword->word = "[";
		return(myword);
		break;
	case ']':
		myword->typenum = 29;
		myword->word = "]";
		return(myword);
		break;
	case '{':
		myword->typenum = 30;
		myword->word = "{";
		return(myword);
		break;
	case '}':
		myword->typenum = 31;
		myword->word = "}";
		return(myword);
		break;
	case ',':
		myword->typenum = 32;
		myword->word = ",";
		return(myword);
		break;
	case ':':
		Obtain2ch();
		if (ch == '=')
		{
			myword->typenum = 18;
			myword->word = ":=";
			return(myword);
			break;
		}
		else
                {
                        retract();
                        myword->typenum = 33;
                        myword->word = ":";
                        return(myword);
                        break;
                }
        case ';':
                myword->typenum = 34;
                myword->word = ";";
                return(myword);
                break;
	case '>':
		Obtain2ch();
		if (ch == '=')
		{
			myword->typenum = 37;
			myword->word = ">=";
			return(myword);
			break;
		}
		retract();
		myword->typenum = 35;
		myword->word = ">";
		return(myword);
		break;
	case '<':
		Obtain2ch();
		if (ch == '=')
		{
			myword->typenum = 38;
			myword->word = "<=";
			return(myword);
			break;
		}
		else if(ch == '<')
                {
                        myword->typenum = 42;
			myword->word = "<<";
			return(myword);
			break;
                }
                else
                {
                        retract();
                        myword->typenum = 36;
                        myword->word = "<";
                        return (myword);
                }
	case '!':
		Obtain2ch();
		if (ch == '=')
		{
			myword->typenum = 40;
			myword->word = "!=";
			return(myword);
			break;
		}
		retract();
		myword->typenum = -1;
		myword->word = "ERROR";
		return(myword);
		break;
        case ' " ':
                myword->typenum = 41;
		myword->word = " \" ";
		return(myword);
		break;
	case '\0':
		myword->typenum = 1000;
		myword->word = "OVER";
		return(myword);
		break;
        case '#':
                myword->typenum = 0;
                myword->word = "#";
                return (myword);
                break;
	default:
		myword->typenum = -1;
		myword->word = "ERROR";
		return(myword);
		break;
	}
}

