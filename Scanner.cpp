#include <bits/stdc++.h>
#define _KEY_WORDEND "waiting for your expanding"
using namespace std;
typedef struct //�ʵĽṹ����Ԫ����ʽ�������ֱ𣬵��������ֵ��
{
	int typenum; //�����ֱ�
	char * word;
}WORD;
char input[255];
char token[255] = "";
int p_input; //ָ��
int p_token;
char ch;
char * rwtab[] = { "if","then","else","end","repeat","until","read","write","int","main","float","double","return",_KEY_WORDEND };
/*
char * rwtab[] = { "if","then","else","end","repeat","until","read","write","while","do","int","main",
                        "float","double","return","cout",_KEY_WORDEND };
*/

WORD * scanner();//ɨ��

//�����뻺������ȡһ���ַ���ch��
char Obtain2ch()
{
	ch = input[p_input];
	p_input++;
	return ch;
}

//ȥ���հ׷���
void deleteblank()
{
	while (ch == ' ' || ch == 10)
	{
		ch = input[p_input];
		p_input++;
	}
}

// ƴ�ӵ���
void split()
{
    if (p_token >= 10000) {
        // ���ʻ���������������
        printf("Error: token buffer overflow\n");
        exit(1);
    }

    token[p_token] = ch;
    p_token++;
    token[p_token] = '\0';
}

//�ж��Ƿ���ĸ
int letter()
{
	if (ch >= 'a'&&ch <= 'z' || ch >= 'A'&&ch <= 'Z')
		return 1;
	else
		return 0;
}

//�ж��Ƿ�����
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

//�����ؼ��ֱ��
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
	return 10;//������ǹؼ��֣��򷵻��ֱ���10
}

//����һ���ַ�
void retract()
{
	p_input--;
}

int main()
{
	int over = 1;
	WORD* oneword = new WORD;

	//ʵ�ִ��ļ���ȡ�����
	cout << "read something from input.txt" << endl;

	FILE *fp;

	fp = fopen("token.txt", "w"); // ���ļ�
	fputs("", fp); // д������

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
                                                FILE* fp = fopen("token.txt", "a+"); // ���ļ�
                                                if (fp == NULL) {
                                                    printf("�޷����ļ�\n");
                                                    return 1;
                                                }
                                                fputs(oneword->word, fp); // д������
                                                fputs(" ", fp); // д������
                                                fclose(fp); // �ر��ļ�
                                                printf("������д���ļ�\n");
                                                                                    }

                                }
                                over = oneword->typenum;
                        }
                        scanf("%[^#]s", input);
                }
        }
    char tokendone[100000];
    FILE *p=fopen("token.txt","r");
    fscanf(p,"%[^#]",tokendone);//#����������ļ��в�����ַ�
    fclose(p);
    int j=0;
    for(int i=0;i<strlen(tokendone);i++){
        if(i==strlen(tokendone)-1) continue;//ɾ���ո�ͻس�
        tokendone[j++]=tokendone[i];
    }
    tokendone[j]=0;//0�� NULL
    p=fopen("tokendone.txt","w");//�½�һ��in2.txt�ļ�
    fprintf(p,"%s",tokendone);//�������ļ�����in2.txt��
    fclose(p);
    /*
    char filename[] = "token.txt";
    char buffer[100];
    int file_size, i;

    // ���ļ�
    fp = fopen(filename, "r+");
    if (fp == NULL) {
        printf("�޷����ļ� %s\n", filename);
        exit(1);
    }

    // ��ȡ�ļ���С
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);

    // ���ļ�ָ���ƶ��������ڶ����ַ�
    fseek(fp, -2, SEEK_END);

    // ��ȡ���һ���ַ�
    fgets(buffer, 2, fp);

    // ������һ���ַ��ǻ��з������ļ�ָ���ٴ���ǰ�ƶ�һλ
    if (strcmp(buffer, "\n") == 0) {
        fseek(fp, -1, SEEK_END);
        file_size--;
    }

    // ���ļ�ָ���ƶ��������ڶ����ַ�
    fseek(fp, -2, SEEK_END);

    // �����һ���ַ��滻Ϊ���ַ�
    fputc('\0', fp);

    // �ر��ļ�
    fclose(fp);

    printf("�ɹ�ɾ���ļ� %s �����һ���ַ�\n", filename);
    */
    return 0;
}

//�ʷ�ɨ�����
WORD * scanner()
{
	WORD * myword = new WORD;
	myword->typenum = 10;  //��ʼֵ
	myword->word = "";
	p_token = 0;   //���ʻ�����ָ��
	Obtain2ch();
	deleteblank();//ȥ���հ�

    if (letter())
    {
        // �����ȡ�����ַ�����ĸ������ѭ��
        while (letter() || digit())
        {
            split(); // �����ַ�
            Obtain2ch(); // ������ȡ��һ���ַ�
        }
        retract(); // ����һ���ַ�����Ϊ����ȡ���ַ�������ĸ������
        myword->typenum = reserve(); // �жϵ����Ƿ�Ϊ�ؼ��֣�������Ӧ���ֱ���
        myword->word = token; // �����ʴ���ṹ����
        return myword; // ���ؽṹ��ָ��
    }
	else if (digit())  //�ж϶�ȡ���ĵ������ַ�������
	{
		while (digit()) //����������������
		{
			split();
			Obtain2ch();
		}
		retract();
		//���ֵ����ֱ���ͳһΪ20�����������ֵΪ���ֱ���
		myword->typenum = 20;
		myword->word = token;
		return(myword);
	}
	else switch (ch)
	{
	case '=':
		Obtain2ch();//���ַ�Ϊ=,�ٶ�ȡ��һ���ַ��ж�
		if (ch == '=')
		{
			myword->typenum = 39;
			myword->word = "==";
			return(myword);
		}
		retract();//��ȡ�����¸��ַ�����=����Ҫ���ˣ�ֱ�����=
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
        case '/'://��ȡ���÷���֮��Ҫ�ж���һ���ַ���ʲô���ţ��ж��Ƿ�Ϊע��
                Obtain2ch();//���ַ�Ϊ/,�ٶ�ȡ��һ���ַ��ж�
		if (ch == '*') // ˵����ȡ������ע��
		{
		        Obtain2ch();

			while(ch != '*')
                        {
                                Obtain2ch();//ע��û����֮ǰһֱ��ȡע�ͣ��������
                                if(ch == '*')
                                {
                                        Obtain2ch();
                                        if(ch == '/')//ע�ͽ���
                                        {
                                                myword->typenum = 999;
                                                myword->word = "ע��";
                                                return (myword);
                                                break;
                                        }
                                }

                        }

		}
                else
                {
                        retract();//��ȡ�����¸��ַ�����*��������ע�ͣ���Ҫ���ˣ�ֱ�����/

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

