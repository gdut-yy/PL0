/*** PL0 COMPILER WITH CODE GENERATION ***/ 
//--------------------------------------------------------------------------- 
#include <vcl.h> 
#pragma hdrstop 
#include "Unit1.h" 
//--------------------------------------------------------------------------- 
#pragma package(smart_init) 
#pragma resource "*.dfm" 
TForm1 *Form1; 
//--------------------------------------------------------------------------- 
const  AL    =  10;  /* LENGTH OF IDENTIFIERS */ 
const  NORW  =  28;  /* # OF RESERVED WORDS */ 
const  TXMAX = 100;  /* LENGTH OF IDENTIFIER TABLE */ 
const  NMAX  =  14;  /* MAX NUMBER OF DEGITS IN NUMBERS */ 
const  AMAX  =2047;  /* MAXIMUM ADDRESS */ 
const  RMAX  =32767; 
const  LEVMAX=   3;  /* MAX DEPTH OF BLOCK NESTING */ 
const  CXMAX = 200;  /* SIZE OF CODE ARRAY */ 
 
//.............. 
int UPARR;       //数组上界的全局变量 
int DOWNARR;     //数组下界的全局变量 
int WNUMK=0;     //实数与整型区别的全局变量 
 
//.............. 
 
typedef enum  { 
NUL, IDENT, PLUS, MINUS, TIMES,
SLASH, ODDSYM, EQL, NEQ, LSS, LEQ, GTR, GEQ,
BECOMES, BEGINSYM, ENDSYM, IFSYM, THENSYM,
WHILESYM, WRITESYM, READSYM, DOSYM, CALLSYM,
CONSTSYM, VARSYM, PROCSYM, PROGSYM,

PP,MM,PE,ME,	// ++,--,+=,-=
OO,REALS,INTS,CHARS, SQLPAREN, 
SQRPAREN,LPAREN, RPAREN, COMMA, SEMICOLON, PERIOD,
ELSESYM,REPEATSYM,UNTILSYM,ARRAYSYM,OFSYM, 
FUNCSYM ,FORSYM,TOSYM, 
DOWNTOSYM,INTEGERSYM,REALSYM,CHARSYM,DIVSYM,MODSYM 
        } SYMBOL; 
char *SYMOUT[] = {"NUL",       "IDENT",    "PLUS",      "MINUS",     "TIMES", 
                  "PP",        "MM",       "OO",        "REALS",     "INTS", 
                  "CHARS",     "SLASH",    "ODDSYM",    "EQL",       "NEQ", 
                  "LSS",       "LEQ",      "GTR",       "PE",        "LPAREN", 
                  "RPAREN",    "COMMA",    "SEMICOLON", "PERIOD",    "GEQ", 
                  "SQLPAREN",  "SQRPAREN", "BECOMES",   "BEGINSYM",  "ENDSYM", 
                  "IFSYM",     "THENSYM",  "ELSESYM",   "REPEATSYM", "ME", 
                  "WHILESYM",  "WRITESYM", "READSYM",   "DOSYM",     "CALLSYM", 
                  "UNTILSYM",  "FORSYM",   "TOSYM",     "DOWNTOSYM", "CONSTSYM", 
                  "VARSYM",    "PROCSYM",  "PROGSYM",   "FUNCSYM",   "ARRAYSYM", 
                  "OFSYM",     "INTEGERSYM", "REALSYM", "CHARSYM",   "DIVSYM", 
                  "MODSYM" }; 
typedef  int *SYMSET; // SET OF SYMBOL; 
typedef  char ALFA[11]; 
typedef  enum { CONSTANT,PROCEDUR,ARR,VARIABLE,FUNCTION,PARR} OBJECTS ; 
typedef  enum { CHARCON, REALCON, INTCON,NOTYP} TYPES; 
typedef  enum { LIT,OPR,LOD,STO,STOARR,LODARR,CAL,INI,JMP,JPC,STOVAR,LODVAR,STOPAR}FCT;  //加入对数组的运算STOARR和LODARR,加入对地址参数的运算，STOVAR和LODVAR 
typedef struct { 
	 FCT F;     /*FUNCTION CODE*/ 
	 int L; 	/*0..LEVMAX  LEVEL*/ 
	 float A;     /*0..AMAX    DISPLACEMENT ADDR*/ 
} INSTRUCTION; 
	  /* LIT O A -- LOAD CONSTANT A             */ 
	  /* OPR 0 A -- EXECUTE OPR A               */ 
	  /* LOD L A -- LOAD VARIABLE L,A           */ 
	  /* STO L A -- STORE VARIABLE L,A          */ 
	  /* CAL L A -- CALL PROCEDURE A AT LEVEL L */ 
	  /* INI 0 A -- INCREMET T-REGISTER BY A    */ 
	  /* JMP 0 A -- JUMP TO A                   */ 
	  /* JPC 0 A -- JUMP CONDITIONAL TO A       */ 
char   CH;  /*LAST CHAR READ*/ 
SYMBOL SYM; /*LAST SYMBOL READ*/ 
ALFA   ID;  /*LAST IDENTIFIER READ*/ 
float  NUM; /*LAST NUMBER READ*/ 
int    CC;  /*CHARACTER COUNT*/ 
int    LL;  /*LINE LENGTH*/ 
int    CX;  /*CODE ALLOCATION INDEX*/ 
char   LINE[81]; 
INSTRUCTION  CODE[CXMAX]; 
ALFA    KWORD[NORW+1]; 
SYMBOL  WSYM[NORW+1]; 
SYMBOL  SSYM['^'+1]; 
ALFA    MNEMONIC[14]; 
 
SYMSET  DECLBEGSYS, STATBEGSYS, FACBEGSYS; 
 
//.............. 
int PARACOUNT;                //参数的个数 
ALFA FUNLAYER[LEVMAX+1];      //存放当前所处理函数的函数名 
 
struct DS // DS STORE         //Display函数活动区域记录 
{ 
  int tp; 
  int bp; 
  //int index; 
} DISPLAY[LEVMAX+1]; 
//.............. 
 
struct { 
  ALFA NAME; 
  OBJECTS KIND; 
  TYPES TYPE; 
  //--------- 
  int  CS; 
  bool IsVAR; 
  //--------- 
  union { 
    float VAL;   /*CONSTANT*/ 
    struct { int LEVEL,ADR,SIZE; } vp;  /*VARIABLE,PROCEDUR:*/ 
  }; 
} TABLE[TXMAX]; 
 
FILE *FIN,*FOUT; 
int ERR; 
 
void EXPRESSION(SYMSET FSYS, int LEV, int &TX); 
void TERM(SYMSET FSYS, int LEV, int &TX); 
//--------------------------------------------------------------------------- 
int SymIn(SYMBOL SYM, SYMSET S1) { 
  return S1[SYM]; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetUnion(SYMSET S1, SYMSET S2) { 
  SYMSET S=(SYMSET)malloc(sizeof(int)*56); 
  for (int i=0; i<56; i++) 
	if (S1[i] || S2[i]) S[i]=1; 
	else S[i]=0; 
  return S; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetAdd(SYMBOL SY, SYMSET S) { 
  SYMSET S1; 
  S1=(SYMSET)malloc(sizeof(int)*56); 
  for (int i=0; i<56; i++) S1[i]=S[i]; 
  S1[SY]=1; 
  return S1; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetNew(SYMBOL a) { 
  SYMSET S; int i,k; 
  S=(SYMSET)malloc(sizeof(int)*56); 
  for (i=0; i<56; i++) S[i]=0; 
  S[a]=1; 
  return S; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetNew(SYMBOL a, SYMBOL b) { 
  SYMSET S; int i,k; 
  S=(SYMSET)malloc(sizeof(int)*56); 
  for (i=0; i<56; i++) S[i]=0; 
  S[a]=1;  S[b]=1; 
  return S; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c) { 
  SYMSET S; int i,k; 
  S=(SYMSET)malloc(sizeof(int)*56); 
  for (i=0; i<56; i++) S[i]=0; 
  S[a]=1;  S[b]=1; S[c]=1; 
  return S; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d) { 
  SYMSET S; int i,k; 
  S=(SYMSET)malloc(sizeof(int)*56); 
  for (i=0; i<56; i++) S[i]=0; 
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; 
  return S; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e) { 
  SYMSET S; int i,k; 
  S=(SYMSET)malloc(sizeof(int)*56); 
  for (i=0; i<56; i++) S[i]=0; 
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1; 
  return S; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e, SYMBOL f) { 
  SYMSET S; int i,k; 
  S=(SYMSET)malloc(sizeof(int)*56); 
  for (i=0; i<56; i++) S[i]=0; 
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1; S[f]=1; 
  return S; 
} 
//--------------------------------------------------------------------------- 
SYMSET SymSetNULL() { 
  SYMSET S; int i,n,k; 
  S=(SYMSET)malloc(sizeof(int)*56); 
  for (i=0; i<56; i++) S[i]=0; 
  return S; 
} 
//--------------------------------------------------------------------------- 
void Error(int n) { 
  String s = "***"+AnsiString::StringOfChar(' ', CC-1)+"^"; 
  Form1->printls(s.c_str(),n);   fprintf(FOUT,"%s%d\n", s.c_str(), n); 
  ERR++; 
} /*Error*/ 
//--------------------------------------------------------------------------- 
void RunTimeError(int n,int T) 
{ 
   String f; 
   switch(n) 
  { 
    case 210:  f="Runtime error 210 at ""00x"+IntToStr(T)+"^"; break; 
  } 
  Form1->printfs(f.c_str());   fprintf(FOUT,"%s\n", f.c_str()); 
  ERR++; 
 
 
} 
 
//---------------------------------------------------------------------------- 
void GetCh() { 
  if (CC==LL) { 
    if (feof(FIN)) { 
	  Form1->printfs("PROGRAM INCOMPLETE"); 
	  fprintf(FOUT,"PROGRAM INCOMPLETE\n"); 
	  fclose(FOUT); 
	  exit(0); 
	} 
	LL=0; CC=0; 
	CH=' '; 
	while (!feof(FIN) && CH!=10) 
      { CH=fgetc(FIN);  LINE[LL++]=CH; } 
	LINE[LL-1]=' ';  LINE[LL]=0; 
    String s=IntToStr(CX); 
    while(s.Length()<3) s=" "+s; 
    s=s+" "+LINE; 
	Form1->printfs(s.c_str()); 
    fprintf(FOUT,"%s\n",s); 
  } 
  CH=LINE[CC++]; 
} /*GetCh()*/ 
//--------------------------------------------------------------------------- 
void GetSym() { 
  int i,J,K;   ALFA  A; 
  while (CH<=' ') GetCh(); 
  if ((CH>='A' && CH<='Z')||(CH>='a' && CH<='z')) { /*ID OR RESERVED WORD*/ 
    K=0; 
	do { 
	  if (K<AL) A[K++]=CH; 
	  GetCh(); 
	}while((CH>='A' && CH<='Z')||(CH>='a' && CH<='z')||(CH>='0' && CH<='9')); 
	A[K]='\0'; 
	strcpy(ID,A); i=1; J=NORW; 
	do { 
	  K=(i+J) / 2; 
	  if (strcmp(ID,KWORD[K])<=0) J=K-1; 
	  if (strcmp(ID,KWORD[K])>=0) i=K+1; 
	}while(i<=J); 
	if (i-1 > J) SYM=WSYM[K]; 
	else SYM=IDENT; 
	 
  } 
  else 
    if (CH>='0' && CH<='9') { /*NUMBER*/ 
      K=0; NUM=0; SYM=INTS;  float count=10; 
	  do 
      { 
		  if(CH=='.') 
		  { 
			  GetCh(); 
			  SYM=REALS; 
			  while(CH>='0' && CH<='9') 
			  { 
				  NUM=NUM+(CH-'0')/count; 
				  K++; 
				  count*=10; 
				  GetCh(); 
			  } 
			  break; 
		  } 
		  else 
		  { 
			  NUM=10*NUM+(CH-'0'); 
			  K++; 
			  GetCh(); 
		  } 
      }while((CH>='0' && CH<='9')||CH=='.'); 
	  if (K>NMAX) Error(30); 
    } 
    else 
    //----------------------------加入字符类型 只接受 'a' 里面的一个字母 
      if((int)CH==39) 
      { 
         GetCh(); 
         if((CH>='A' && CH<='Z')||(CH>='a' && CH<='z')) 
         { 
            NUM=(int)CH; 
            GetCh(); 
            if((int)CH==39) 
              SYM=CHARS; 
            else { NUM=0;SYM=NUL;Error(39);}  //类型错误 
         } 
         else Error(39); //类型不匹配 
         GetCh(); 
      } 
      else 
 
 
    //-----------------------------------------------------------end 
    //++++++++++++++++++++++++++++START 
      if (CH==':') { 
	    GetCh(); 
		if (CH=='=') { SYM=BECOMES; GetCh(); } 
		else 
                  { 
                   SYM=OO; 
                   } 
      } 
    //+++++++++++++++++++++++END 
	  else /* THE FOLLOWING TWO CHECK WERE ADDED 
	         BECAUSE ASCII DOES NOT HAVE A SINGLE CHARACTER FOR <= OR >= */ 
	    if (CH=='<') { 
		  GetCh(); 
		  if (CH=='=') { SYM=LEQ; GetCh(); } 
                   else if(CH=='>'){SYM=NEQ;GetCh();} 
		    else SYM=LSS; 
		} 
		else 
		  if (CH=='>') { 
		    GetCh(); 
			if (CH=='=') { SYM=GEQ; GetCh(); } 
			else SYM=GTR; 
          } 
//+++++++++++++++++++++++++++++++添加的 ++ += -- -= 标识符 
                 else if(CH=='+') 
                        { 
                          GetCh(); 
                          if(CH=='=') 
                            { 
                              SYM=PE; 
                              GetCh(); 
                            } 
                            else if(CH=='+') 
                                  { 
                                    SYM=PP; 
                                    GetCh(); 
                                  }else 
                                     SYM=PLUS; 
                        } 
                 else if(CH=='-') 
                        { 
                          GetCh(); 
                          if(CH=='=') 
                            { 
                              SYM=ME; 
                              GetCh(); 
                            } 
                            else if(CH=='-') 
                                  { 
                                    SYM=MM; 
                                    GetCh(); 
                                  }else 
                                     SYM=MINUS; 
                        } 
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++添加结束 
		  else { SYM=SSYM[CH]; GetCh(); } 
} /*GetSym()*/ 
//--------------------------------------------------------------------------- 
void GEN(FCT X, int Y, float Z) { 
  if (CX>CXMAX) { 
    Form1->printfs("PROGRAM TOO LONG"); 
	fprintf(FOUT,"PROGRAM TOO LONG\n"); 
	fclose(FOUT); 
    exit(0); 
  } 
  CODE[CX].F=X; CODE[CX].L=Y; CODE[CX].A=Z; 
  CX++; 
} /*GEN*/ 
//--------------------------------------------------------------------------- 
void TEST(SYMSET S1, SYMSET S2, int N) { 
  if (!SymIn(SYM,S1)) { 
    Error(N); 
	while (!SymIn(SYM,SymSetUnion(S1,S2))) GetSym(); 
  } 
} /*TEST*/ 
//--------------------------------------------------------------------------- 
void ENTER(OBJECTS K,TYPES TYP, int LEV, int &TX, int &DX,ALFA IDS,bool IsVAR) 
{ /*ENTER OBJECT INTO TABLE*/               //这里入表函数的处理过程，参量分别是  K OBJECTS 
                                            //ALFA NUMKIND 是该标识符的类型， 
                                            //ALFA IDS 标识符的名   //bool IsVAR是否变量参数还是值参数 
  TX++; 
  strcpy(TABLE[TX].NAME,IDS); TABLE[TX].KIND=K; 
  TABLE[TX].TYPE=TYP; 
  TABLE[TX].IsVAR=IsVAR; 
  switch (K) { 
	case CONSTANT: 
	       if (NUM>AMAX && SYM==INTEGERSYM) { Error(31); NUM=0; } 
               if (NUM>RMAX && SYM==REALSYM) { Error(31); NUM=0;} 
	       TABLE[TX].VAL=NUM; 
               break; 
	case PROCEDUR: 
               TABLE[TX].vp.LEVEL=LEV; 
	       break; 
//++++++++++++++++++++++++++++++++++++++++++++++++数组输入到符号表 
        case ARR: 
               TABLE[TX].vp.LEVEL=LEV;            //数组的层 
               TABLE[TX].vp.ADR=DX;               //数组的首地址 
               TX++; 
               TABLE[TX].VAL=UPARR;               //开多一个符号表，存地址的上界 
               TX++; 
               TABLE[TX].VAL=DOWNARR;             //再开多个符号表，存地址的下界 
               DX=DX+DOWNARR-UPARR+1; 
               break; 
//+++++++++++++++++++++++++++++++++++++++++++++++++end arr enter 
//------------------------------------------------函数标识符输入到符号表 
	case FUNCTION: 
		TABLE[TX].vp.LEVEL=LEV; 
                if(strcmp(TABLE[TX].NAME,"")!=0) 
                { 
                   TABLE[TX].vp.ADR=DX; 
                   DX++; 
                   TX++; 
                   TABLE[TX].VAL=PARACOUNT;        //存入定义参数的个数 
                } 
		break; 
//-------------------------------------------------end  fun enter 
 
	    default: 
	       TABLE[TX].vp.LEVEL=LEV; TABLE[TX].vp.ADR=DX; DX++; 
	       break; 
 
  } 
 
 
 
} /*ENTER*/ 
//--------------------------------------------------------------------------- 
int POSITION(ALFA ID, int TX) { /*FIND IDENTIFIER IN TABLE*/ 
  int i=TX; 
  strcpy(TABLE[0].NAME,ID); 
  while (strcmp(TABLE[i].NAME,ID)!=0) i--; 
  return i; 
} /*POSITION*/ 
 
 
//--------------------------------------------------------------------------- 
void ConstDeclaration(int LEV,int &TX,int &DX) { 
 
  if (SYM==IDENT) 
  { 
 
      GetSym(); 
      if (SYM==EQL||SYM==BECOMES) 
	  { 
	     if (SYM==BECOMES) Error(1); 
	     GetSym(); 
             switch(SYM) 
             { 
               case INTS: ENTER(CONSTANT,INTCON,LEV,TX,DX,ID,FALSE); GetSym(); break; 
               case REALS : ENTER(CONSTANT,REALCON,LEV,TX,DX,ID,FALSE); GetSym(); break; 
               case CHARS : ENTER(CONSTANT,CHARCON,LEV,TX,DX,ID,FALSE); GetSym(); break; 
                default : Error(2); break; 
             } 
    } 
    else Error(3); 
  } 
  else Error(4); 
} /*ConstDeclaration()*/ 
//--------------------------------------------------------------------------- 
 
//+++++++++++++++++++++++++++start para declaration 
void ParaDeclaration(int LEV,int &TX,int &DX,int count,ALFA VARS[],bool IsVAR) 
                                         //函数参数变量声明过程 
                                         //int count   同一类型变量的个数 
                                         //ALFA VARS[] 存有同一类型变量临时数组 
{ 
 
 
        //if(IsVAR) LEV--; 
        TYPES tm; 
	if(SYM==ARRAYSYM)                //数组的参数的处理 
	{ 
		GetSym(); 
		if(SYM==OFSYM) 
		{ 
			UPARR=0; 
			DOWNARR=0; 
                        GetSym(); 
		        if(SYM==INTEGERSYM || SYM==REALSYM || SYM==CHARSYM) 
			{ 
                                 switch(SYM) 
                                 { 
                                   case INTEGERSYM: tm=INTCON;  break; 
                                   case REALSYM   : tm=REALCON; break; 
                                   case CHARSYM   : tm=CHARCON; break; 
                                 } 
                                for(int i=0; i<count; i++) 
                                { 
         			    ENTER(PARR,tm,LEV,TX,DX,VARS[i],IsVAR);    //定义数组 
                                    DISPLAY[LEV].bp++; 
                                } 
			} 
			else Error(4); 
		}else Error(92);  //不正确的数组定义 
	}else 
              if(SYM==INTEGERSYM || SYM==REALSYM || SYM==CHARSYM) 
	        { 
                                 switch(SYM) 
                                 { 
                                   case INTEGERSYM: tm=INTCON;  break; 
                                   case REALSYM   : tm=REALCON; break; 
                                   case CHARSYM   : tm=CHARCON; break; 
                                 } 
 
			for(int i=0;i<count;i++) 
			{ 
                            ENTER(VARIABLE,tm,LEV,TX,DX,VARS[i],IsVAR);    //定义变量 
                            DISPLAY[LEV].bp++; 
			} 
 
		} 
		else Error(4); 
 
} 
 
//+++++++++++++++++++++++++++++++++++++++++++++++++++end par declaration 
 
//----------------++++++++++++++--------ParaGetSub------- 
void ParaGetSub(SYMSET FSYS,int LEV,int &TX,int Pos) 
{ 
	int count=0;                         //调用函数之前的参数处理过程，pos为该函数在符号表的位置 
        int i; 
        GetSym(); 
        if(SYM==LPAREN)                      //函数参数的语法形式 标识符（参数1，参数2，参数3。。); 
	{ 
		GetSym(); 
                count=TABLE[Pos+1].VAL;       //count 是由该函数原先存在数组表的个数 
		Pos-=count; 
                 
                while(count)                  //这里是对原先存入的符号表的所有参数进行匹配处理，少了是不可以的 
                { 
                     if(TABLE[Pos].KIND==FUNCTION) Error(73);     //这里是在调用时的参数还未匹配完，就已经到尽头了， 
                                                                  //所以出错，多了是不可以的 
                     else 
                     { 
                         if(TABLE[Pos].IsVAR)                    //匹配的如果是变量参数，即运算时是对实在参数的运算 
                         { 
                            i=POSITION(ID,TX);                   //i为形参所在的符号表下标 
                            if(i==0) Error(11); 
                            else 
                            { 
                               if(TABLE[Pos].TYPE==TABLE[i].TYPE) //类型匹配 
                               { 
                                 GEN(LIT,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);  //开始实现传递地址 
                                 GEN(STOPAR,0,/*DISPLAY[LEV].tp+*/TABLE[Pos].vp.ADR);          //直接存入 
                               } 
                               else Error(32); 
                               count--; 
                               GetSym(); 
                                     if(count) 
                                     { 
                                       if(SYM==COMMA)                           //参数之间用逗号格开 
                                          GetSym(); 
                                        else Error(48); 
                                     } 
                            } 
                         } 
                         else                                                   //不是变量参数，就为值参数 
                         { 
                                   if(TABLE[Pos].KIND==ARR)                      //如果类型配为数组 ,需要在此函数的数据活动开始区域， 
                                                                                //新开空间存放数组变量 
                                   { 
                                     i=POSITION(ID,TX);                         //i为形参所在的符号表下标 
                                     if(TABLE[i].KIND!=PARR) Error(32); 
                                     else 
                                     { 
                                         int UP,DOWN; 
                                         TABLE[Pos].vp.ADR=DISPLAY[LEV].bp;          //新开空间的首地址，为display中存的地址 
                                         DOWN=TABLE[Pos-1].VAL=TABLE[i].VAL;        //新开的数组空间调试时有致命错误导致整个程序被关， 
 
                                         UP=TABLE[Pos-2].VAL=TABLE[i].VAL; 
                                         int LASTDX=DOWN-UP+1; 
                                         for(int j=1;j<=LASTDX;j++) 
                                         { 
                                           GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR+j);       //一个一个的将原数组值对应复制到 
                                                                                                  //新开的数据区中，以便以后调用 
                                           GEN(STOPAR,0,TABLE[Pos].vp.ADR+j); 
                                           DISPLAY[LEV+1].tp++; 
                                         } 
                                         count--; 
                                         GetSym(); 
                                         if(count) 
                                         { 
                                           if(SYM==COMMA) 
                                           GetSym(); 
                                           else Error(48); 
                                         } 
                                         else Error (32); 
                                     } 
                                   } 
                                   else if(TABLE[Pos].KIND==VARIABLE)                //值参数匹配为其它变量时， 
                                   { 
                                       if(TABLE[Pos].TYPE==CHARCON )     //匹配为字符 
                                       { 
                                         if(SYM!=CHARS)  Error(32); 
                                         else 
                                         { 
                                            GEN(LIT,0,(int)CH);                      //直接存入 
                                            GEN(STOPAR,0,TABLE[Pos].vp.ADR); 
                                         } 
                                       } 
                                       else 
                                       {                                                  //匹配为数字型 
                                            if(TABLE[Pos].TYPE==INTCON)                  //如果是整型 
                                            WNUMK=1;                                      //在进行运算表达式时就只能进行1类运算 
                                            EXPRESSION(SymSetUnion(SymSetNew(RPAREN,COMMA),FSYS),LEV,TX); //值参数传递的如果是表达式 
                                                                                                          //例如 A（s+1+2*3); 
                                            WNUMK=0; 
                                            GEN(STOPAR,0,/*DISPLAY[LEV].tp+*/TABLE[Pos].vp.ADR);          //直接存入 
                                       } 
                                       count--; 
                                       if(count) 
                                       { 
                                            if(SYM==COMMA) 
                                            GetSym(); 
                                            else Error(48); 
                                       } 
                                   }//end if(VARIABLE) 
                                 //}//end else(i=0) 
                         }//end else VAR 
                     }//else if(function) 
                     Pos++;           //匹配下一个参数变量 
                }//end while(count) 
            if (SYM!=RPAREN) Error(33); 
         }//end if(SYM==LPAREN) 
}//end   ParaGetSub 
 
//++++++++++++++++++++++++++start var &arr 
void VarDeclaration(int LEV,int &TX,int &DX,int count,ALFA VARS[])  //变量类型声明处理 
{ 
	int i=0; 
        TYPES tm; 
    if(SYM==ARRAYSYM)           //如果是数组，按照PASCAL数组定义的语法处理 
                                // <标识符> : ARRAY[整数..整数] OF <类型> 
    { 
           GetSym(); 
	   if(SYM==SQLPAREN) //SQLPAREN 代表 '[' 符号 
       	   { 
	     GetSym(); 
	     if(SYM==REALS)                                       //上界为数字，实际上读入的是一个整型和一个点， 
                                                                  // 词法分析的时候就认为是实数了，不过不带小数。 
 
             { 
                if(NUM-(int)NUM!=0) Error(93); //类型错误     //判读是否带有小数，带小数就出错 
                else 
	        UPARR=(int)NUM; 
	     }else 
	     if(SYM==IDENT)     //上界为常数 
	     { 
	        int i=POSITION(ID,TX); 
	        if (i==0) 
	        { 
	      	  Error(11); 
	      	  UPARR=0; 
	        } 
	        else 
	        if(TABLE[i].KIND==CONSTANT && TABLE[i].TYPE==INTCON) 
                                                               //strcmp(TABLE[ii].NUMKIND,"INTEGER")==0  这里就不是过滤上面的错误 
                                                               //是检测常数的数据类型是否为整型,字符和实型均不可接受 
         	{ 
	        	UPARR=TABLE[i].VAL; 
	     	} 
	        else 
		{ 
		  UPARR=0; 
		  Error(91);  //存在于表中，但非常数 
		} 
	    }//if上界 
	    GetSym(); 
	    if(SYM==PERIOD) 
	    { 
	    	GetSym(); 
            }else 
	    Error(92);  //不正确的数组定义 
            if(SYM==INTS )    //下界为数字 
	    { 
	    	DOWNARR=(int)NUM+1; 
	    }else 
	    if(SYM==IDENT)     //下界为常数 
	    { 
	    	int ii=POSITION(ID,TX); 
	    	if (ii==0) 
	        { 
		   Error(11); 
                   DOWNARR=0; 
                } 
		else 
		if(TABLE[ii].KIND==CONSTANT && TABLE[ii].TYPE==INTCON) 
                                                                        //是检测常数的数据类型是否为整型,字符和实型均不可接受 
		{ 
		   DOWNARR=TABLE[ii].VAL; 
		} 
		else 
		{ 
		   DOWNARR=0; 
		   Error(91);  //存在于表中，但非常数 
		} 
            }//if下界 
            GetSym(); 
	    if(SYM==SQRPAREN)        //SQRPAREN 代表 ']'符号 
	    { 
	      	GetSym(); 
	      	if(SYM==OFSYM) 
	      	GetSym(); 
	      	else Error(93); 
            } 
	    else Error(93);  //数组定义不正确 
	    if(SYM==INTEGERSYM  || SYM==REALSYM || SYM==CHARSYM ) 
	    { 
                     switch(SYM) 
                     { 
                         case INTEGERSYM: tm=INTCON;  break; 
                         case REALSYM   : tm=REALCON; break; 
                         case CHARSYM   : tm=CHARCON; break; 
                     } 
 
	    	     for(i=0;i<count;i++) 
	             ENTER(ARR,tm,LEV,TX,DX,VARS[i],FALSE);    //定义数组 
          }else Error(92); 
        }else Error(92); 
    } 
    else 
    if(SYM==INTEGERSYM  || SYM==REALSYM || SYM==CHARSYM ) 
    { 
                switch(SYM) 
                { 
                    case INTEGERSYM: tm=INTCON;  break; 
                    case REALSYM   : tm=REALCON; break; 
                    case CHARSYM   : tm=CHARCON; break; 
                } 
 
          for(i=0;i<count;i++) 
          ENTER(VARIABLE,tm,LEV,TX,DX,VARS[i],FALSE);    //定义变量 
    } 
    else Error(4); 
     GetSym(); 
 
} /*VarDeclaration()*/ 
 
//++++++++++++++++++++++++++end 
 
 
//--------------------------------------------------------------------------- 
void ListCode(int CX0) 
{                                                 /*LIST CODE GENERATED FOR THIS Block*/ 
  if (Form1->ListSwitch->ItemIndex==0) 
    for (int i=CX0; i<CX; i++) 
    { 
      String s=IntToStr(i); 
      while(s.Length()<3)s=" "+s; 
      if(CODE[i].A-(int)CODE[i].A!=0) 
      { 
      s=s+" "+MNEMONIC[CODE[i].F]+" "+IntToStr(CODE[i].L)+" "+IntToStr((int)CODE[i].A)+"."+IntToStr((int)((CODE[i].A-(int)CODE[i].A)*10000)); 
      fprintf(FOUT,"%3d%5s%4d%4.4f\n",i,MNEMONIC[CODE[i].F],CODE[i].L,CODE[i].A); 
      } 
      else 
      { 
      s=s+" "+MNEMONIC[CODE[i].F]+" "+IntToStr(CODE[i].L)+" "+IntToStr((int)CODE[i].A); 
      fprintf(FOUT,"%3d%5s%4d%4d\n",i,MNEMONIC[CODE[i].F],CODE[i].L,(int)CODE[i].A); 
      } 
	  Form1->printfs(s.c_str()); 
	 // fprintf(FOUT,"%3d%5s%4d%4f\n",i,MNEMONIC[CODE[i].F],CODE[i].L,CODE[i].A); 
    } 
} /*ListCode()*/ 
 
//-------------+++++++++++++++---------数组偏移地址处理过程 
void ARRGetSub(SYMSET FSYS, int LEV,int &TX) 
{ 
 
      GetSym(); 
      if(SYM==SQLPAREN)        //数组的语法为  <数组名> [ 运算表达式 ]; 
        { 
          GetSym(); 
 
          WNUMK=1;                 //因为数组中只能进行,整型的运算,所以对下面的运算只能调用一类运算,WNUMK是全局变量 
          EXPRESSION(SymSetUnion(SymSetNew(SQRPAREN),FSYS),LEV,TX); 
          WNUMK=0; 
 
          //GetSym(); 
          if(SYM!=SQRPAREN) 
          Error(93);  //数组定义不正确 
 
        } 
        else Error(93); 
        //if(arr) 
} 
 
 
//----------------++++++++++++++--------------- 
 
 
 
//--------------------------------------------------------------------------- 
void FACTOR(SYMSET FSYS, int LEV, int &TX) { 
  int i,jk,tm; 
  jk=0; 
  tm=0; 
  FCT STOMode[3]={STO,STOARR,STOVAR};   //这里是后面加入的对数组的运算实现代码 STOARR和LODARR 
  FCT LODMode[3]={LOD,LODARR,LODVAR}; 
  TEST(FACBEGSYS,FSYS,24); 
  while (SymIn(SYM,FACBEGSYS)) { 
	if (SYM==IDENT) { 
	  i=POSITION(ID,TX); 
	  if (i==0) Error(11); 
          else if((TABLE[i].TYPE==REALCON && WNUMK==1)||TABLE[i].TYPE==CHARCON) 
                Error(32);//类型不匹配,字符型不可以参加运算 
          else 
		switch (TABLE[i].KIND) { 
		  case CONSTANT: GEN(LIT,0,TABLE[i].VAL); break; 
		  case VARIABLE: 
                                 if(TABLE[i].IsVAR) 
                                 { 
                                  GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                                  jk=2; 
                                  tm=1; 
                                 } 
                                 GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
                                   break; 
		  case PROCEDUR: Error(21); break; 
                  //------------------------------------------------------------在表达式中的函数计算 
                  case FUNCTION: 
                        ParaGetSub(FSYS,LEV,TX,i);              //函数参数处理过程调用 
                        GEN(CAL,LEV-TABLE[i].vp.LEVEL,TABLE[i].CS);      //生成调用函数的代码 
                        GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);   //读出函数的返回值,我是直接把函数的返回值放在函数名变量所在的地址位置. 
                       break; 
                  //----------------------------------------------------------- 
                  case ARR:                                                //数组处理 
                        jk=1; 
                        ARRGetSub(FSYS,LEV,TX);                           //调用数组的偏移地址处理 
                        GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//读出数组元素的值，放入栈顶 
                        break; 
		} 
          GetSym(); 
          //++++++++++++++++++++ START ++ --       //变量自加运算 
          if(SYM==PP) 
          { 
             GetSym(); 
             GEN(LIT,0,1); 
             GEN(OPR,0,2); 
             GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
             GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
          } 
          else if(SYM==MM)                    //变量自减运算 
          { 
            GetSym(); 
            GEN(LIT,0,1); 
            GEN(OPR,0,3); 
            GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
            GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
          } 
                   //++++++++++++++++++++ end ++ -- 
	} 
	else 
	  if (SYM==INTS || SYM==REALS) { 
		if (NUM>AMAX && SYM==INTS) { Error(31); NUM=0; } 
                if (NUM>RMAX && SYM==REALS){ Error(31); NUM=0;} 
                if (WNUMK && SYM==REALS) { Error(32); NUM=0;}  //类型不匹配 
		GEN(LIT,0,NUM); GetSym(); 
                if(SYM==PP)       //数字自加运算 
                { 
                   GetSym(); 
                   GEN(LIT,0,1); 
                   GEN(OPR,0,2); 
                } 
                else if(SYM==MM)   //数字自减运算 
                { 
                   GetSym(); 
                   GEN(LIT,0,1); 
                   GEN(OPR,0,3); 
                } 
	  } 
	  else 
		if (SYM==LPAREN) { 
		  GetSym(); EXPRESSION(SymSetAdd(RPAREN,FSYS),LEV,TX); 
		  if (SYM==RPAREN) GetSym(); 
		  else Error(22); 
		} 
	  TEST(FSYS,FACBEGSYS,23); 
 
  } 
 
}/*FACTOR*/ 
//--------------------------------------------------------------------------- 
void TERM(SYMSET FSYS, int LEV, int &TX) {  /*TERM*/ 
  SYMBOL MULOP; 
  FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH,DIVSYM,MODSYM)), LEV,TX); 
  while (SYM==TIMES || SYM==SLASH || SYM==DIVSYM || SYM==MODSYM) 
  { 
	MULOP=SYM;  GetSym(); 
	FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH,DIVSYM,MODSYM)),LEV,TX); 
	switch(MULOP) 
	{ 
	        case TIMES:  GEN(OPR,0,4);  break; 
		case SLASH:  if(WNUMK) Error(18); else GEN(OPR,0,5);  break;   //整型运算中不允许出现 '/'运算符 
		case DIVSYM: GEN(OPR,0,17); break;    //整型的运算符，整除 
		case MODSYM: GEN(OPR,0,18); break;    //取余预算 
	} 
 
  } 
} /*TERM*/ 
//--------------------------------------------------------------------------- 
void EXPRESSION(SYMSET FSYS, int LEV, int &TX) { 
  SYMBOL ADDOP; 
 
  if (SYM==PLUS || SYM==MINUS) { 
    ADDOP=SYM; GetSym(); 
    TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX); 
    if (ADDOP==MINUS) GEN(OPR,0,1); 
  } 
       else TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX); 
  while (SYM==PLUS || SYM==MINUS) { 
    ADDOP=SYM; GetSym(); 
    TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX); 
    if (ADDOP==PLUS) GEN(OPR,0,2); 
    else GEN(OPR,0,3); 
 
  } 
} /*EXPRESSION*/ 
//--------------------------------------------------------------------------- 
void CONDITION(SYMSET FSYS,int LEV,int &TX) { 
  SYMBOL RELOP; 
  if (SYM==ODDSYM) { GetSym(); EXPRESSION(FSYS,LEV,TX); GEN(OPR,0,6); } 
  else { 
	EXPRESSION(SymSetUnion(SymSetNew(EQL,NEQ,LSS,LEQ,GTR,GEQ),FSYS),LEV,TX); 
	if (!SymIn(SYM,SymSetNew(EQL,NEQ,LSS,LEQ,GTR,GEQ))) Error(20); 
	else { 
	  RELOP=SYM; GetSym(); EXPRESSION(FSYS,LEV,TX); 
	  switch (RELOP) { 
	    case EQL: GEN(OPR,0,8);  break; 
	    case NEQ: GEN(OPR,0,9);  break; 
	    case LSS: GEN(OPR,0,10); break; 
	    case GEQ: GEN(OPR,0,11); break; 
	    case GTR: GEN(OPR,0,12); break; 
	    case LEQ: GEN(OPR,0,13); break; 
	  } 
	} 
  } 
} /*CONDITION*/ 
//--------------------------------------------------------------------------- 
void STATEMENT(SYMSET FSYS,int LEV,int &TX) {   /*STATEMENT*/ 
  int i,CX1,CX2,jk,tm; 
  FCT STOMode[3]={STO,STOARR,STOVAR}; 
  FCT LODMode[3]={LOD,LODARR,LODVAR}; 
  bool IsFUN; 
  SYMBOL Temp; 
  jk=0; 
  tm=0; 
  switch (SYM) { 
	case IDENT: 
//+++++++++++++START ++ 
                IsFUN=FALSE; 
		i=POSITION(ID,TX); 
		if (i==0) Error(11); 
		else 
                  { 
                     if (TABLE[i].KIND==CONSTANT)  /*ASSIGNMENT TO NON-VARIABLE*/ 
                      { 
                        Error(12); 
                        i=0; 
                      } 
                      //call function in statement or assimenet to function.val 
                       if(TABLE[i].KIND==FUNCTION) 
                       { 
                          ParaGetSub(FSYS,LEV,TX,i); 
                         if((LEV==0 && strcmp(FUNLAYER[LEV],ID)!=0)||SYM==RPAREN)//call 
                                                                           //函数调用不是第一层而且与当前函数不同名，就可以调用， 
                                                                          //否则就是用下面的赋值功能，作为函数返回值 
                         { 
                            if(strcmp(FUNLAYER[LEV],ID)==0) 
                              GEN(CAL,0,TABLE[i].CS); 
                            else 
                             GEN(CAL,LEV-TABLE[i].vp.LEVEL,TABLE[i].CS); 
                             GetSym(); 
                            break; 
                         } 
                         IsFUN=TRUE; 
                         goto LOOP; 
                         //else assiment 
                       } 
                      //----------------end call function 
                      //call procedure 
                     if(TABLE[i].KIND==PROCEDUR) 
                     { 
                        ParaGetSub(FSYS,LEV,TX,i); 
                        GEN(CAL,LEV-TABLE[i].vp.LEVEL,TABLE[i].CS); 
                        GetSym(); 
                        break; 
                     } 
                     //------------------end call procedure 
                     if(TABLE[i].KIND==ARR)           //处理的变量是数组 
                     { 
                       ARRGetSub(FSYS,LEV,TX); 
                       jk=1; 
                     } 
                     if(TABLE[i].IsVAR)              //处理变量是否为按地址传递的参数 
                     { 
                        GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                        jk=2; 
                        tm=1; 
                     } 
                  }//else if(i==0) 
 
                if(TABLE[i].TYPE==CHARCON) 
                { 
                   GetSym(); 
                   if(SYM!=BECOMES)  Error(13); 
                    else 
                    { 
                      GetSym(); 
                      if(SYM!=CHARS) Error(32); //类型出错 
                      else 
                      { 
                        GEN(LIT,0,NUM); 
                        GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
                      } 
                    } 
                    GetSym(); 
                    break; 
 
                } 
                GetSym(); 
		if (SYM==BECOMES) 
                 { 
LOOP:                  if(TABLE[i].TYPE==INTCON) 
                       WNUMK=1; 
	               GetSym(); 
                       EXPRESSION(FSYS,LEV,TX); 
                       if (i!=0) GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
                       else Error(98); 
                       WNUMK=0; 
                       if(IsFUN)  GEN(OPR,0,0); 
                } 
 
  else 
    if(SYM==PP)                 //++运算 
      { 
        GetSym(); 
 
        GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
        GEN(LIT,0,1); 
        GEN(OPR,0,2); 
        GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
      } 
 
  else 
    if(SYM==PE)                  //+=运算 
      { 
 
        GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
         if(TABLE[i].TYPE==INTCON) 
           WNUMK=1; 
        GetSym(); 
        EXPRESSION(FSYS,LEV,TX); 
        WNUMK=0; 
        GEN(OPR,0,2); 
        GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
      } 
 
  else 
    if(SYM==MM)             //--运算 
      { 
        GetSym(); 
        GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
        GEN(LIT,0,1); 
        GEN(OPR,0,3); 
        GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
      } 
 
  else 
    if(SYM==ME)                //-=运算 
      { 
         
        GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
         if(TABLE[i].TYPE==INTCON) 
           WNUMK=1; 
        GetSym(); 
        EXPRESSION(FSYS,LEV,TX); 
        WNUMK=0; 
        GEN(OPR,0,3); 
        GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL+tm,TABLE[i].vp.ADR); 
      } 
 
      else Error(13); 
      break; 
//+++++++++++++++++++++END 
	case READSYM: 
		GetSym(); 
		if (SYM!=LPAREN) Error(34); 
		else 
		  do { 
                       	GetSym(); 
			if (SYM==IDENT) i=POSITION(ID,TX); 
			else i=0; 
			if (i==0) Error(35); 
			else 
			{ 
                                if(TABLE[i].KIND==ARR)     //如果读入到数组元素里 
                                { 
                                 ARRGetSub(FSYS,LEV,TX); 
                                 jk=1;                     //存储或读取模式选1 
                                 } 
                                 if(TABLE[i].IsVAR)              //处理变量是否为按地址传递的参数 
                                 { 
                                    GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                                    jk=2; 
                                 } 
 
				if(TABLE[i].TYPE==INTCON ) 
					GEN(OPR,0,16);           //16号运算代码为输入整型 
                                else if( TABLE[i].TYPE==CHARCON) 
                                        GEN(OPR,0,20);            //20号为输入字符 
				else GEN(OPR,0,15);               //15为输入实型 
                                GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
 
			} 
			GetSym(); 
		  }while(SYM==COMMA); 
		if (SYM!=RPAREN) { 
		  Error(33); 
		  while (!SymIn(SYM,FSYS)) GetSym(); 
		} 
		else GetSym(); 
		break; /* READSYM */ 
	case WRITESYM: 
		GetSym(); 
		if (SYM==LPAREN) { 
		  do { 
                        int boolen=1; 
			GetSym(); 
                        if(SYM==IDENT) 
                        { 
                          int i=POSITION(ID,TX); 
                          if(i!=0 && TABLE[i].TYPE==CHARCON) 
                          { 
                            if(TABLE[i].KIND==ARR) { ARRGetSub(FSYS,LEV,TX);jk=1;} 
                            if(TABLE[i].IsVAR)              //处理变量是否为按地址传递的参数 
                            { 
                               GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                               jk=2; 
                            } 
                           GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                           GEN(OPR,0,19);          //19号为输出字符 
                           GetSym(); 
                           boolen=0; 
                          } 
                         } 
                        if(boolen){ 
			EXPRESSION(SymSetUnion(SymSetNew(RPAREN,COMMA),FSYS),LEV,TX); 
			GEN(OPR,0,14);       //14号输出运算 
                        } 
		  }while(SYM==COMMA); 
		  if (SYM!=RPAREN) Error(33); 
		  else GetSym(); 
		} 
		//GEN(OPR,0,15); 
		break; /*WRITESYM*/ 
 
       //++++++++++++++++++++++++++++START   IF语句的分析过程 
	case IFSYM:                         //PASCAL语法 为  IF <条件> THEN <语句> ELSE <语句> 
		GetSym();                  //条件判断开始 
		CONDITION(SymSetUnion(SymSetNew(THENSYM,DOSYM),FSYS),LEV,TX); 
		if (SYM==THENSYM) 
                   GetSym(); 
		  else 
                    Error(16); 
		CX1=CX;                    //记录当前代码段到CX1后处理 THEN 后的语句 
                GEN(JPC,0,0); 
		 STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ELSESYM),FSYS),LEV,TX); 
                if(SYM==ELSESYM)          //如果后面出现ELSE 
                  { 
                    GetSym(); 
                    CX2=CX;               //记录当前代码段到CX2 
                    GEN(JMP,0,0); 
                    CODE[CX1].A=CX;       //回填当前代码段作为已记录CX1的JPC代码的跳转地址 
                    STATEMENT(FSYS,LEV,TX); //ELSE后的语句 
                        CODE[CX2].A=CX;      //回填当前代码段为已记录CX2的JMP代码的跳转地址 
 
                  } 
                  else 
                  { 
                      CODE[CX1].A=CX;     //不出现ELSE，就只回填当前代码段作为已记录CX1的JPC代码的跳转地址 
                  } 
		break; 
          //+++++++++++++++++++++++++END */ 
       	case BEGINSYM: 
		GetSym(); 
		STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX); 
		while (SymIn(SYM, SymSetAdd(SEMICOLON,STATBEGSYS))) { 
		  if (SYM==SEMICOLON) GetSym(); 
		  else Error(10); 
		  STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX); 
		} 
		if (SYM==ENDSYM) GetSym(); 
		else Error(17); 
		break; 
    	case WHILESYM: 
		CX1=CX; GetSym(); CONDITION(SymSetAdd(DOSYM,FSYS),LEV,TX); 
		CX2=CX; GEN(JPC,0,0); 
		if (SYM==DOSYM) GetSym(); 
		else Error(18); 
		STATEMENT(FSYS,LEV,TX); 
		GEN(JMP,0,CX1); 
		CODE[CX2].A=CX; 
		break; 
        //+++++++++++++++START   repeatsym 
        case REPEATSYM: 
		CX1=CX; 
                GetSym(); 
                STATEMENT(SymSetUnion(SymSetNew(UNTILSYM),FSYS),LEV,TX); 
                if (SYM==UNTILSYM) GetSym(); 
		else Error(98); 
		CONDITION(FSYS,LEV,TX); 
		GEN(JPC,0,CX1); 
		break; 
        case FORSYM:     //FOR语句的处理过程 
        GetSym();        //PASCAL语法  FOR <变量> := <运算表达式> TO|DOWNTO <运算表达式> DO 
        if(SYM==IDENT) 
        { 
		i=POSITION(ID,TX); 
		if(i==0) Error(11); 
		else 
		{           //变量为常数或过程都出错 
			if(TABLE[i].KIND==CONSTANT||TABLE[i].KIND==PROCEDUR) 
			{ 
				Error(12); 
				i=0; 
			} 
			if(TABLE[i].KIND==ARR) 
			{ 
				ARRGetSub(FSYS,LEV,TX); 
                                jk=0; 
 
			} 
                        if(TABLE[i].IsVAR)              //处理变量是否为按地址传递的参数 
                        { 
                            GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                            jk=2; 
                        } 
		}//else if(i==0) 
 
		GetSym(); 
		if(SYM==BECOMES)  //变量赋初始值 
		{ 
		        GetSym();        //第一个运算表达式 
			EXPRESSION(SymSetUnion(SymSetNew(TOSYM,DOWNTOSYM),FSYS),LEV,TX); 
 
 
		} 
                if(SYM==TOSYM)    //如果为TO 
		{ 
			CX1=CX;   //CX1记录当前代码段作为开始循环位置 
                        GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);  //把数存进变量中 
                        GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);  //读取此变量的数，放入栈顶 
                        GetSym(); 
                        EXPRESSION(SymSetUnion(SymSetNew(DOSYM),FSYS),LEV,TX);   //计算第二个运算表达式，放入栈顶 
                        GEN(OPR,0,13);                                           //判断运算是否大于 
                        CX2=CX; GEN(JPC,0,0);       //CX2记录当前代码段，用于JPC的跳转地址回填 
                        Temp=TOSYM; 
		} 
		else if(SYM==DOWNTOSYM)       //DOWNTO 类似 TO，不同处为判断预算选是否小于 
		{ 
			CX1=CX; 
                        GEN(STOMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                        GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); 
                        GetSym(); 
                        EXPRESSION(SymSetUnion(SymSetNew(DOSYM),FSYS),LEV,TX); 
                        GEN(OPR,0,11); 
			CX2=CX; GEN(JPC,0,0); 
                        Temp=DOWNTOSYM; 
		} 
		else Error(19); 
 
		if(SYM==DOSYM) GetSym();     //做DO后面的<语句> 
		else Error(18); 
		STATEMENT(FSYS,LEV,TX); 
 
                GEN(LODMode[jk],LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);   //读取变量的值,放入栈顶 
                GEN(LIT,0,1); //步长为1 
                switch(Temp) 
                { 
		  case DOWNTOSYM: GEN(OPR,0,3); break;   //DOWNTOSYM 就减 1 
                  case TOSYM: GEN(OPR,0,2); break;       //TOSYM 就加 1 
                } 
 
 
		GEN(JMP,0,CX1);          //无条件跳转到CX1记录的地址段 
		CODE[CX2].A=CX;          //回填JPC的跳转地址 
                
	}// if(SYM) 
	else Error(97); 
               break; 
        //+++++++++++++++END 
   } 
  TEST(FSYS,SymSetNULL(),19); 
} /*STATEMENT*/ 
//--------------------------------------------------------------------------- 
void Block(int LEV, int TX, SYMSET FSYS) { 
  int DX=3;    /*DATA ALLOCATION INDEX*/ 
  int TX0=TX;  /*INITIAL TABLE INDEX*/ 
  int CX0=CX;  /*INITIAL CODE INDEX*/ 
 
  ALFA VARS[500]; 
  DX+=DISPLAY[LEV].bp; 
 
  int count; 
 
  TABLE[TX].vp.ADR=CX; GEN(JMP,0,0); 
  if (LEV>LEVMAX) Error(32); 
  do { 
    if (SYM==CONSTSYM) { 
      GetSym(); 
      do { 
        ConstDeclaration(LEV,TX,DX); 
        while (SYM==COMMA) { 
          GetSym();  ConstDeclaration(LEV,TX,DX); 
        } 
        if (SYM==SEMICOLON) GetSym(); 
        else Error(5); 
      }while(SYM==IDENT); 
    } 
    if (SYM==VARSYM)      //变量类型声明 依PASCAL语法为 <变量1>,<变量2>...<变量n>:<类型名>; 
    { 
       GetSym(); 
      do 
	  { 
        //VarDeclaration(LEV,TX,DX); 
        count=0; 
		           if(SYM==IDENT) 
			   { 
					  strcpy(VARS[count],ID);  //同一类型的变量都存入,临时变量队列中 
					  count++; 
					  GetSym(); 
					  if(count>500) Error(5); 
                           } 
                           else Error(4); 
		  while (SYM==COMMA) 
                  { 
                              GetSym(); 
                              if(SYM==IDENT) 
		              strcpy(VARS[count],ID); 
                              else Error(5); 
                              count++; 
                              GetSym(); 
                              if(count>500) Error(5); 
                  } 
		  if(SYM==OO) 
		  { 
			  GetSym(); 
                          VarDeclaration(LEV,TX,DX,count,VARS); 
		  }else Error(4); 
//----------------------------------------------------------------- 
		  if (SYM==SEMICOLON) GetSym(); 
	               else Error(6); 
      }while(SYM==IDENT); 
    } 
    while ( SYM==PROCSYM) { 
      GetSym(); 
	  if (SYM==IDENT) { ENTER(PROCEDUR,NOTYP,LEV,TX,DX,ID,FALSE); 
                            TABLE[TX-1].CS=CX; GetSym(); } 
	  else Error(4); 
	  if (SYM==SEMICOLON) GetSym(); 
	  else Error(5); 
	  Block(LEV+1,TX,SymSetAdd(SEMICOLON,FSYS)); 
	  if (SYM==SEMICOLON) { 
	    GetSym(); 
	    TEST(SymSetUnion(SymSetNew(IDENT,PROCSYM),STATBEGSYS),FSYS,6); 
	  } 
	  else Error(5); 
    } 
 
//------------------------------------------------------------------- 
    while( SYM==FUNCSYM)         //函数定义 依PASCAL语法为 FUNCTION <函数名>(参数,..参数:<类型名>;参数:<类型名>):<类型名> 
	{                                                             //    |(VAR 参数:<类型名>...):<类型名> 
		bool IsVAR=FALSE;    //参数是值参还是变量参数 
                int  DXP=3;            //参数的相对地址个数 
		PARACOUNT=0; 
 
		ALFA FUNNAME; 
		GetSym(); 
		if(SYM==IDENT) 
                { 
                    strcpy(FUNNAME,ID); 
                    strcpy(FUNLAYER[LEV+1],ID);                    //当前处理的函数名，全局变量 
                    GetSym(); 
                    ENTER(FUNCTION,NOTYP,LEV,TX,DX,"",FALSE);     //参数对列的队尾 
 
                } 
		else Error(4); 
		if(SYM==LPAREN)                                   //参数定义类似变量定义 
		{ 
			GetSym(); 
                        DISPLAY[LEV+1].bp=0; 
			do 
			{ 
				//ParaVarDeclaration(LEV,TX,DX); 
				count=0;                          //同一类的参数个数，如a，b，c：integer 
				IsVAR=FALSE;                      //如果识别到VAR 
                                if(SYM==VARSYM) 
			        { 
    		         	      GetSym(); 
				      IsVAR=TRUE; 
		        	} 
				if(SYM==IDENT) 
				{ 
					strcpy(VARS[count],ID);    //先存入在名字表中 
					count++; 
                                        PARACOUNT++;               //同类变量增加，变量总数增加 
					GetSym(); 
					if(count>500) Error(5); 
				} 
				else Error(4); 
				while (SYM==COMMA)                 //同类变量用‘，’号隔开 
				{ 
					GetSym(); 
                                        if(SYM==IDENT) 
					strcpy(VARS[count],ID);      //先存入在名字表中 
                                        else Error(5); 
					count++;                     //同类变量增加，变量总数增加 
                                        PARACOUNT++; 
				        GetSym(); 
				    if(count>500) Error(5); 
				} 
				if(SYM==OO) 
				{ 
				    GetSym(); 
 
                                    ParaDeclaration(LEV+1,TX,DXP,count,VARS,IsVAR);   //从一次定义完名字表中的同类变量 
 
                                    GetSym(); 
				}else Error(4); 
                                if(SYM!=RPAREN) 
                                { 
               				if (SYM==SEMICOLON) GetSym(); 
	                                  else Error(6); 
                                 } 
			}while(SYM==IDENT); 
 
			if(SYM==RPAREN) GetSym(); 
			else Error(4); 
		}//end if(SYM==LPAREN); 
		if(SYM==OO) 
		{ 
			GetSym(); 
			if(SYM==INTEGERSYM || SYM==REALSYM ||SYM==CHARSYM ) 
			{ 
                                TYPES tm; 
                                switch(SYM) 
                                { 
                                  case INTEGERSYM: tm=INTCON;  break; 
                                  case REALSYM   : tm=REALCON; break; 
                                  case CHARSYM   : tm=CHARCON; break; 
                                } 
				ENTER(FUNCTION,tm,LEV,TX,DX,FUNNAME,FALSE);    //定义函数变量  作为参数队列的队首 
                                TABLE[TX-1].CS=CX; 
				GetSym(); 
			} 
		        else Error(4); 
		}else Error(4); 
 
                if (SYM==SEMICOLON) GetSym(); 
	        else Error(5); 
	        Block((LEV+1),TX,SymSetAdd(SEMICOLON,FSYS));   //递归调用入口函数 BLOCK 
	        if (SYM==SEMICOLON) 
                { 
	           GetSym(); 
	           TEST(SymSetUnion(SymSetNew(IDENT,FUNCSYM),STATBEGSYS),FSYS,6); 
	        } 
	        else Error(5); 
	}//end while(SYM==FUNSYM) 
             
 
 
 
 
//-------------------------------------------------------------------- 
 
    TEST(SymSetAdd(IDENT,STATBEGSYS), DECLBEGSYS,7); 
  }while(SymIn(SYM,DECLBEGSYS)); 
  CODE[TABLE[TX0].vp.ADR].A=CX; 
  TABLE[TX0].vp.ADR=CX;   /*START ADDR OF CODE*/ 
  TABLE[TX0].vp.SIZE=DX;  /*SIZE OF DATA SEGMENT*/ 
  GEN(INI,0,DX); 
  DISPLAY[LEV+1].tp=0; 
  STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX); 
  GEN(OPR,0,0);  /*RETURN*/ 
  TEST(FSYS,SymSetNULL(),8); 
  ListCode(CX0); 
} /*Block*/ 
//--------------------------------------------------------------------------- 
int BASE(int L,int B,float S[]) { 
  int B1=B; /*FIND BASE L LEVELS DOWN*/ 
  while (L>0) { B1=S[B1]; L=L-1; } 
  return B1; 
} /*BASE*/ 
//--------------------------------------------------------------------------- 
void Interpret() { 
  const STACKSIZE = 500; 
  int P,B,T; 		/*PROGRAM BASE TOPSTACK REGISTERS*/ 
  INSTRUCTION I; 
  float S[STACKSIZE];  	/*DATASTORE*/   //把数据栈都定义为实型运算 
  Form1->printfs("~~~ RUN PL0 ~~~"); 
  fprintf(FOUT,"~~~ RUN PL0 ~~~\n"); 
  T=0; B=1; P=0; 
  S[1]=0; S[2]=0; S[3]=0; 
  do { 
    I=CODE[P]; P=P+1; 
    switch (I.F) { 
      case LIT: T++; S[T]=I.A; break; 
	  case OPR: 
	    switch ((int)I.A) { /*OPERATOR*/ 
	      case 0: /*RETURN*/ T=B-1; P=S[T+3]; B=S[T+2]; break; 
	      case 1: S[T]=-S[T];  break; 
	      case 2: T--; S[T]=S[T]+S[T+1];   break; 
	      case 3: T--; S[T]=S[T]-S[T+1];   break; 
	      case 4: T--; S[T]=S[T]*S[T+1];   break; 
	      case 5: T--; S[T]=S[T]/S[T+1];   break; 
	      case 6: S[T]=((int)S[T]%2!=0);   break; 
	      case 8: T--; S[T]=S[T]==S[T+1];  break; 
	      case 9: T--; S[T]=S[T]!=S[T+1];  break; 
	      case 10: T--; S[T]=S[T]<S[T+1];   break; 
	      case 11: T--; S[T]=S[T]>=S[T+1];  break; 
	      case 12: T--; S[T]=S[T]>S[T+1];   break; 
	      case 13: T--; S[T]=S[T]<=S[T+1];  break; 
 
	      case 14: 
 
                  fprintf(FOUT,"%f\n",S[T]); 
                  Form1->printrs("",S[T]); 
                T--; 
                break; 
 
 
	      case 15: //输入实数 
			       T++;  S[T]=InputBox("输入","请键盘输入：", 0).ToDouble(); 
                   Form1->printrs("? ",S[T]); fprintf(FOUT,"? %f\n",S[T]); 
		           break; 
 
 
 
	      case 16: T++;  S[T]=InputBox("输入","请键盘输入：", 0).ToDouble();  //输入整型 
                   if((S[T]-(int)S[T])!=0) { RunTimeError(210,T); goto LOOP;} 
                   Form1->printrs("? ",S[T]); fprintf(FOUT,"? %f\n",S[T]); 
 
		           break; 
             
		  case 17:T--; S[T]=(int)(S[T]/S[T+1]);  break; 
		  case 18:T--; S[T]=(int)S[T]%(int)S[T+1]; break; 
                  case 19: 
                       Form1->printcs(S[T]); 
 
                       fprintf(FOUT,"%c\n",(char)S[T]); T--; 
                   break; 
 
                  case 20:                                   //输入字符 
		    T++; 
                    AnsiString css=InputBox("输入","请键盘输入：", 0);//.AnsiLastChar(); 
                    char *buf=css.c_str(); 
                    S[T]=(int)buf[0]; Form1->printcs('?'); 
                   Form1->printcs((char)S[T]); fprintf(FOUT,"? %c\n",(char)S[T]); 
		           break; 
 
	    } 
	    break; 
      case LOD: T++; S[T]=S[BASE(I.L,B,S)+(int)I.A]; break; 
      case STO: S[BASE(I.L,B,S)+(int)I.A]=S[T]; T--; break; 
 
      case STOPAR:    S[T+(int)I.A]=S[T]; T--; break; 
      case STOARR:    S[BASE(I.L,B,S)+(int)I.A+(int)S[(T-1)]]=S[T]; T--;  break;     //对数组的存储 S[T-1]放入了偏移地址 
      case LODARR:    S[T]=S[BASE(I.L,B,S)+(int)I.A+(int)S[T]]; break;             //对数组的读取 第二个S[T]是放入偏移地址的 
      case LODVAR:    S[T]=S[BASE(I.L,B,S)+(int)S[T]]; break;                    //对地址变量的读取 
      case STOVAR:    S[BASE(I.L,B,S)+(int)S[(T-1)]]=S[T]; T--; T--;  break;      //对地址变量的存储 
      //case LODPARX: 
      //case STOPARX: 
	  case CAL: /*GENERAT NEW Block MARK*/ 
	      S[T+1]=BASE(I.L,B,S); S[T+2]=B; S[T+3]=P; 
	      B=T+1; P=I.A; break; 
	  case INI: T=T+I.A;  break; 
	  case JMP: P=I.A; break; 
      case JPC: if (S[T]==0) P=I.A;  T--;  break; 
    } /*switch*/ 
  }while(P!=0); 
LOOP:  Form1->printfs("~~~ END PL0 ~~~"); 
  fprintf(FOUT,"~~~ END PL0 ~~~\n"); 
} /*Interpret*/ 
//--------------------------------------------------------------------------- 
void __fastcall TForm1::ButtonRunClick(TObject *Sender) { 
  for (CH=' '; CH<='^'; CH++) SSYM[CH]=NUL; 
  for (int i=0;i<3;i++) 
  DISPLAY[i].tp=DISPLAY[i].bp=0;      //DS STORE 
  strcpy(KWORD[ 1],"ARRAY"); 
  strcpy(KWORD[ 2],"BEGIN");       strcpy(KWORD[ 3],"CALL");   //call不用了，函数或过程直接就可以调用了 
  strcpy(KWORD[ 4],"CHAR");        strcpy(KWORD[ 5],"CONST"); 
  strcpy(KWORD[ 6],"DIV");         strcpy(KWORD[ 7],"DO"); 
  strcpy(KWORD[ 8],"DOWNTO");      strcpy(KWORD[ 9],"ELSE"); 
  strcpy(KWORD[10],"END");         strcpy(KWORD[11],"FOR"); 
  strcpy(KWORD[12],"FUNCTION");    strcpy(KWORD[13],"IF"); 
  strcpy(KWORD[14],"INTEGER");     strcpy(KWORD[15],"MOD"); 
  strcpy(KWORD[16],"ODD");         strcpy(KWORD[17],"OF"); 
  strcpy(KWORD[18],"PROCEDURE"); 
  strcpy(KWORD[19],"PROGRAM");     strcpy(KWORD[20],"READ"); 
  strcpy(KWORD[21],"REAL");        strcpy(KWORD[22],"REPEAT"); 
  strcpy(KWORD[23],"THEN");        strcpy(KWORD[24],"TO"); 
  strcpy(KWORD[25],"UNTIL");       strcpy(KWORD[26],"VAR"); 
  strcpy(KWORD[27],"WHILE");       strcpy(KWORD[28],"WRITE"); 
 
  WSYM[ 1]=ARRAYSYM;                                  //新加的关键字都是按字典顺序放的，因为词法分析是用二分查找算法 
  WSYM[ 2]=BEGINSYM;   WSYM[ 3]=CALLSYM; 
  WSYM[ 4]=CHARSYM;    WSYM[ 5]=CONSTSYM; 
  WSYM[ 6]=DIVSYM;     WSYM[ 7]=DOSYM; 
  WSYM[ 8]=DOWNTOSYM;  WSYM[ 9]=ELSESYM; 
  WSYM[10]=ENDSYM;     WSYM[11]=FORSYM; 
  WSYM[12]=FUNCSYM;    WSYM[13]=IFSYM; 
  WSYM[14]=INTEGERSYM; WSYM[15]=MODSYM; 
  WSYM[16]=ODDSYM;     WSYM[17]=OFSYM;  
  WSYM[18]=PROCSYM; 
  WSYM[19]=PROGSYM;    WSYM[20]=READSYM; 
  WSYM[21]=REALSYM;    WSYM[22]=REPEATSYM; 
  WSYM[23]=THENSYM;    WSYM[24]=TOSYM; 
  WSYM[25]=UNTILSYM;   WSYM[26]=VARSYM; 
  WSYM[27]=WHILESYM;   WSYM[28]=WRITESYM; 
 
  SSYM['+']=PLUS;      SSYM['-']=MINUS; 
  SSYM['*']=TIMES;     SSYM['/']=SLASH; 
  SSYM['(']=LPAREN;    SSYM[')']=RPAREN; 
  SSYM['=']=EQL;       SSYM[',']=COMMA; 
  SSYM['.']=PERIOD;    SSYM[';']=SEMICOLON; 
  SSYM['[']=SQLPAREN;   SSYM[']']=SQRPAREN; 
 
  strcpy(MNEMONIC[LIT],"LIT");   strcpy(MNEMONIC[OPR],"OPR"); 
  strcpy(MNEMONIC[LOD],"LOD");   strcpy(MNEMONIC[STO],"STO"); 
  strcpy(MNEMONIC[LODARR],"LODARR");strcpy(MNEMONIC[STOARR],"STOARR"); 
  strcpy(MNEMONIC[LODVAR],"LODVAR");strcpy(MNEMONIC[STOVAR],"STOVAR"); 
  strcpy(MNEMONIC[STOPAR],"STOPAR"); 
  strcpy(MNEMONIC[CAL],"CAL");   strcpy(MNEMONIC[INI],"INI"); 
  strcpy(MNEMONIC[JMP],"JMP");   strcpy(MNEMONIC[JPC],"JPC"); 
 
  DECLBEGSYS=(int*)malloc(sizeof(int)*56); 
  STATBEGSYS=(int*)malloc(sizeof(int)*56); 
  FACBEGSYS =(int*)malloc(sizeof(int)*56); 
 
  for(int j=0; j<56; j++) { 
	DECLBEGSYS[j]=0;  STATBEGSYS[j]=0;  FACBEGSYS[j] =0; 
  } 
 
  DECLBEGSYS[CONSTSYM]=1; 
  DECLBEGSYS[VARSYM]=1; 
  DECLBEGSYS[PROCSYM]=1; 
  DECLBEGSYS[FUNCSYM]=1; 
  DECLBEGSYS[ARR]=1; 
  STATBEGSYS[BEGINSYM]=1; 
  STATBEGSYS[CALLSYM]=1; 
  STATBEGSYS[IFSYM]=1; 
  STATBEGSYS[WHILESYM]=1; 
  STATBEGSYS[WRITESYM]=1; 
  STATBEGSYS[REPEATSYM]=1; 
 
  //------------------------ 
  STATBEGSYS[FORSYM]=1; 
  //---------------------- 
  FACBEGSYS[IDENT] =1; 
  FACBEGSYS[LPAREN]=1; 
  FACBEGSYS[CHARS]=1; 
  FACBEGSYS[INTS]=1; 
  FACBEGSYS[REALS]=1; 
 
 
 
  if ((FIN=fopen((Form1->EditName->Text+".PL0").c_str(),"r"))!=0) { 
	FOUT=fopen((Form1->EditName->Text+".COD").c_str(),"w"); 
    Form1->printfs("=== COMPILE PL0 ==="); 
    fprintf(FOUT,"=== COMPILE PL0 ===\n"); 
	ERR=0; 
	CC=0; CX=0; LL=0; CH=' '; GetSym(); 
	if (SYM!=PROGSYM) Error(0); 
	else { 
	  GetSym(); 
	  if (SYM!=IDENT) Error(0); 
	  else { 
		GetSym(); 
		if (SYM!=SEMICOLON) Error(5); 
		else GetSym(); 
	  } 
	} 
	Block(0,0,SymSetAdd(PERIOD,SymSetUnion(DECLBEGSYS,STATBEGSYS))); 
	if (SYM!=PERIOD) Error(9); 
	if (ERR==0) Interpret(); 
	else { 
	  Form1->printfs("ERROR IN PL/0 PROGRAM"); 
	  fprintf(FOUT,"ERROR IN PL/0 PROGRAM"); 
	} 
	fprintf(FOUT,"\n"); fclose(FOUT); 
  } 
} 
//--------------------------------------------------------------------------- 
 
//--------------------------------------------------------------------------- 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
