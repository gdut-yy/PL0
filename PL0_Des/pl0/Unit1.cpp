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
const  NORW  =  19;  /* # OF RESERVED WORDS */
const  TXMAX = 100;  /* LENGTH OF IDENTIFIER TABLE */
const  NMAX  =  14;  /* MAX NUMBER OF DEGITS IN NUMBERS */
const  AMAX  =2047;  /* MAXIMUM ADDRESS */
const  LEVMAX=   3;  /* MAX DEPTH OF BLOCK NESTING */
const  CXMAX = 200;  /* SIZE OF CODE ARRAY */

const int SYMNUM = 47; //保留字个数

typedef enum  { NUL, IDENT, NUMBER, PLUS, MINUS, TIMES,
	            SLASH, ODDSYM, EQL, NEQ, LSS, LEQ, GTR, GEQ,
	            LPAREN, RPAREN, COMMA, SEMICOLON, PERIOD,
	            BECOMES, BEGINSYM, ENDSYM, IFSYM, THENSYM,
	            WHILESYM, WRITESYM, READSYM, DOSYM, CALLSYM,
	            CONSTSYM, VARSYM, PROCSYM, PROGSYM			
				
				// ↓↓↓ 新增部分 ↓↓↓
                , ELSESYM, FORSYM, STEPSYM, UNTILSYM, RETURNSYM,	// 共5个。ELSE，FOR，STEP，UNTIL，RETURN
                TIMESBECOMES, SLASHBECOMES, ANDSYM, ORSYM, NOTSYM	// 共5个。*=，/=，&，||，！
                , PLUSBECOMES, MINUSBECOMES, INCSYM, DECSYM         // 共4个。+=，-+，++，--
                // ↑↑↑ 新增部分 ↑↑↑
        } SYMBOL;
char *SYMOUT[] = {"NUL", "IDENT", "NUMBER", "PLUS", "MINUS", "TIMES",
	    "SLASH", "ODDSYM", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ",
	    "LPAREN", "RPAREN", "COMMA", "SEMICOLON", "PERIOD",
	    "BECOMES", "BEGINSYM", "ENDSYM", "IFSYM", "THENSYM",
	    "WHILESYM", "WRITESYM", "READSYM", "DOSYM", "CALLSYM",
	    "CONSTSYM", "VARSYM", "PROCSYM", "PROGSYM" 
		
		// ↓↓↓ 新增部分 ↓↓↓
        , "ELSESYM", "FORSYM", "STEPSYM", "UNTILSYM", "RETURNSYM",	// 共5个。ELSE，FOR，STEP，UNTIL，RETURN
        "TIMESBECOMES", "SLASHBECOMES", "ANDSYM", "ORSYM", "NOTSYM"	// 共5个。*=，/=，&，||，！
        , "PLUSBECOMES", "MINUSBECOMES", "INCSYM", "DECSYM"         // 共4个。+=，-+，++，--
        // ↑↑↑ 新增部分 ↑↑↑	
		};
typedef  int *SYMSET; // SET OF SYMBOL;
typedef  char ALFA[11];
typedef  enum { CONSTANT, VARIABLE, PROCEDUR } OBJECTS ;
typedef  enum { LIT, OPR, LOD, STO, CAL, INI, JMP, JPC } FCT;
typedef struct {
	 FCT F;     /*FUNCTION CODE*/
	 int L; 	/*0..LEVMAX  LEVEL*/
	 int A;     /*0..AMAX    DISPLACEMENT ADDR*/
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
int    NUM; /*LAST NUMBER READ*/
int    CC;  /*CHARACTER COUNT*/
int    LL;  /*LINE LENGTH*/
int    CX;  /*CODE ALLOCATION INDEX*/
char   LINE[81];
INSTRUCTION  CODE[CXMAX];
ALFA    KWORD[NORW+1];
SYMBOL  WSYM[NORW+1];
SYMBOL  SSYM['^'+1];
ALFA    MNEMONIC[9];
SYMSET  DECLBEGSYS, STATBEGSYS, FACBEGSYS;

struct {
  ALFA NAME;
  OBJECTS KIND;
  union {
    int VAL;   /*CONSTANT*/
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
//前跟符号集和后跟符号集合成一个集合
SYMSET SymSetUnion(SYMSET S1, SYMSET S2) {
  SYMSET S=(SYMSET)malloc(sizeof(int)*SYMNUM); //为机内码字符集分配空间
  for (int i=0; i<SYMNUM; i++)
	if (S1[i] || S2[i]) S[i]=1;
	else S[i]=0;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetAdd(SYMBOL SY, SYMSET S) {
  SYMSET S1;
  S1=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (int i=0; i<SYMNUM; i++) S1[i]=S[i];
  S1[SY]=1;
  return S1;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (i=0; i<SYMNUM; i++) S[i]=0;
  S[a]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (i=0; i<SYMNUM; i++) S[i]=0;
  S[a]=1;  S[b]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (i=0; i<SYMNUM; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (i=0; i<SYMNUM; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (i=0; i<SYMNUM; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e, SYMBOL f) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (i=0; i<SYMNUM; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1; S[f]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNULL() {
  SYMSET S; int i,n,k;
  S=(SYMSET)malloc(sizeof(int)*SYMNUM);
  for (i=0; i<SYMNUM; i++) S[i]=0;
  return S;
}
//---------------------------------------------------------------------------
void Error(int n) {
  String s = "***"+AnsiString::StringOfChar(' ', CC-1)+"^";
  Form1->printls(s.c_str(),n);   fprintf(FOUT,"%s%d\n", s.c_str(), n);
  ERR++;
} /*Error*/
//---------------------------------------------------------------------------
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
  if (CH>='A' && CH<='Z') { /*ID OR RESERVED WORD*/
    K=0;
	do {
	  if (K<AL) A[K++]=CH;
	  GetCh();
	}while((CH>='A' && CH<='Z')||(CH>='0' && CH<='9'));
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
      K=0; NUM=0; SYM=NUMBER;
	  do {
	    NUM=10*NUM+(CH-'0');
		K++; GetCh();
      }while(CH>='0' && CH<='9');
	  if (K>NMAX) Error(30);
    }
    else
      if (CH==':') {
	    GetCh();
		if (CH=='=') { SYM=BECOMES; GetCh(); }
		else SYM=NUL;
      }
	  else /* THE FOLLOWING TWO CHECK WERE ADDED
	         BECAUSE ASCII DOES NOT HAVE A SINGLE CHARACTER FOR <= OR >= */
	    if (CH=='<') {
		  GetCh();
		  if (CH=='=') { SYM=LEQ; GetCh(); }
		  
		  // ↓↓↓ 新增部分 ↓↓↓	
		  else if(CH=='>') { SYM=NEQ; GetCh(); }	// 不等号运算符 <>
		  // ↑↑↑ 新增部分 ↑↑↑
		  
		  else SYM=LSS;
		}
		else
		  if (CH=='>') {
		    GetCh();
			if (CH=='=') { SYM=GEQ; GetCh(); }
			else SYM=GTR;
          }
		  
	// ↓↓↓ 新增部分 ↓↓↓
    else if(CH=='*') {		
        GetCh();
        if(CH=='=') {           // 运算符 '*='
            SYM=TIMESBECOMES; GetCh(); 
        }
        else SYM=TIMES;
    } else if(CH=='/') {	
        GetCh();
        if(CH=='=') {           // 运算符 '/='
            SYM=SLASHBECOMES; GetCh(); 
        }
        else if(CH=='*'){       // '/* */' 多行注释
            GetCh();
            i=CH;
            while (i!='*' || CH!='/') {
                i=CH;
                GetCh();
            }
            if(i!='*' && CH!='/') Error(19);
            else{
                GetCh();
                GetSym();
            }
        }
        else if(CH=='/'){       // '//' 单行注释
            i=CX;
            while (CC!=LL) {
                GetCh();
            }
            GetSym();
        }
        else SYM=SLASH;
    } else if(CH=='+') {    
        GetCh();
        if(CH=='=') {           // 运算符 '+='
            SYM=PLUSBECOMES; GetCh(); 
        }
        else if(CH=='+') {      // 运算符 '++'
            SYM=INCSYM; GetCh();
        }
        else SYM=PLUS;
    } else if(CH=='-') {
        GetCh();
        if(CH=='=') {           // 运算符 '-='
            SYM=MINUSBECOMES; GetCh(); 
        }
        else if(CH=='-') {      // 运算符 '--'
            SYM=DECSYM; GetCh();
        }
        else SYM=MINUS;
    } else if(CH=='&') {        // 运算符 '&'
        GetCh();
        SYM=ANDSYM;
    } else if(CH=='|') {	
        GetCh();
        if(CH=='|') {           // 运算符 '||'
            SYM=ORSYM; GetCh(); 
        }
        else Error(19);
    } else if(CH=='!') {        // 运算符 '!'
        GetCh();
        SYM=NOTSYM;
    }
    // ↑↑↑ 新增部分 ↑↑↑
	
		  else { SYM=SSYM[CH]; GetCh(); }
} /*GetSym()*/
//---------------------------------------------------------------------------
void GEN(FCT X, int Y, int Z) {
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
void ENTER(OBJECTS K, int LEV, int &TX, int &DX) { /*ENTER OBJECT INTO TABLE*/
  TX++;
  strcpy(TABLE[TX].NAME,ID); TABLE[TX].KIND=K;
  switch (K) {
	case CONSTANT:
	       if (NUM>AMAX) { Error(31); NUM=0; }
	       TABLE[TX].VAL=NUM;
	       break;
    case VARIABLE:
	       TABLE[TX].vp.LEVEL=LEV; TABLE[TX].vp.ADR=DX; DX++;
	       break;
	case PROCEDUR:
	       TABLE[TX].vp.LEVEL=LEV;
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
  if (SYM==IDENT) {
    GetSym();
    if (SYM==EQL||SYM==BECOMES) {
	  if (SYM==BECOMES) Error(1);
	  GetSym();
	  if (SYM==NUMBER) { ENTER(CONSTANT,LEV,TX,DX); GetSym(); }
	  else Error(2);
    }
    else Error(3);
  }
  else Error(4);
} /*ConstDeclaration()*/
//---------------------------------------------------------------------------
void VarDeclaration(int LEV,int &TX,int &DX) {
  if (SYM==IDENT) { ENTER(VARIABLE,LEV,TX,DX); GetSym(); }
  else Error(4);
} /*VarDeclaration()*/
//---------------------------------------------------------------------------
void ListCode(int CX0) {  /*LIST CODE GENERATED FOR THIS Block*/
  if (Form1->ListSwitch->ItemIndex==0)
    for (int i=CX0; i<CX; i++) {
      String s=IntToStr(i);
      while(s.Length()<3)s=" "+s;
      s=s+" "+MNEMONIC[CODE[i].F]+" "+IntToStr(CODE[i].L)+" "+IntToStr(CODE[i].A);
	  Form1->printfs(s.c_str());
	  fprintf(FOUT,"%3d%5s%4d%4d\n",i,MNEMONIC[CODE[i].F],CODE[i].L,CODE[i].A);
    }
} /*ListCode()*/;
//---------------------------------------------------------------------------
void FACTOR(SYMSET FSYS, int LEV, int &TX) {
  int i;
  TEST(FACBEGSYS,FSYS,24);
  while (SymIn(SYM,FACBEGSYS)) {
	if (SYM==IDENT) {
	  i=POSITION(ID,TX);
	  if (i==0) Error(11);
	  else
		switch (TABLE[i].KIND) {
		  case CONSTANT: GEN(LIT,0,TABLE[i].VAL); break;
		  case VARIABLE: GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); break;
		  case PROCEDUR: Error(21); break;
		}
	  GetSym();
	  
	// ↓↓↓ 新增部分 ↓↓↓
    // 因子中的 i++ 和 i-- 运算 实现
    if(SYM==INCSYM||SYM==DECSYM){       
        GEN(LIT,0,1);                   // 将常数 1 放入栈顶
        if(SYM==INCSYM){                // i++ 
            GEN(OPR,0,2);               // 次栈顶 = 次栈顶 + 栈顶
            GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);     // 将栈顶送入变量单元
            GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);     // 将变量送入栈顶
            // 加了 1 的减去 1
            GEN(LIT,0,1);
            GEN(OPR,0,3);
        }else if(SYM==DECSYM){          // i-- 
            GEN(OPR,0,3);               // 次栈顶 = 次栈顶 - 栈顶
            GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);     // 将栈顶送入变量单元
            GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);     // 将变量送入栈顶
            // 减了 1 的加上 1
            GEN(LIT,0,1);
            GEN(OPR,0,2);
        }
        GetSym();
    }
        // ↑↑↑ 新增部分 ↑↑↑
	}
	else
	  if (SYM==NUMBER) {
		if (NUM>AMAX) { Error(31); NUM=0; }
		GEN(LIT,0,NUM); GetSym();
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
  FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH)), LEV,TX);
  while (SYM==TIMES || SYM==SLASH) {
	MULOP=SYM;  GetSym();
	FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH)),LEV,TX);
	if (MULOP==TIMES) GEN(OPR,0,4);
	else GEN(OPR,0,5);
  }
} /*TERM*/;
//---------------------------------------------------------------------------
void EXPRESSION(SYMSET FSYS, int LEV, int &TX) {
  SYMBOL ADDOP;
  int i;    // 新增 int i;
  if (SYM==PLUS || SYM==MINUS) {
    ADDOP=SYM; GetSym();
    TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX);
    if (ADDOP==MINUS) GEN(OPR,0,1);
  }
  
   // ↓↓↓ 新增部分 ↓↓↓
  else if(SYM==INCSYM){     // ++i
      GetSym();
      if(SYM==IDENT){
          i=POSITION(ID,TX);
          if(i==0) Error(11);
          else if(TABLE[i].KIND!=VARIABLE){
              Error(12);
              i=0;
          }
          if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          GEN(LIT,0,1);
          GEN(OPR,0,2);
          if(i!=0){
              GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
              GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          }
          GetSym();
      }
      else Error(45);
  }
  else if(SYM==DECSYM){     // --i
      GetSym();
      if(SYM==IDENT){
          i=POSITION(ID,TX);
          if(i==0) Error(11);
          else if(TABLE[i].KIND!=VARIABLE){
              Error(12);
              i=0;
          }
          if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          GEN(LIT,0,1);
          GEN(OPR,0,3);
          if(i!=0){
              GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
              GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          }
          GetSym();
      }
      else Error(45);
  }
  // ↑↑↑ 新增部分 ↑↑↑
  
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
  int i,CX1,CX2,CX3;
  switch (SYM) {
	case IDENT:
		i=POSITION(ID,TX);
		if (i==0) Error(11);
		else
		  if (TABLE[i].KIND!=VARIABLE) { /*ASSIGNMENT TO NON-VARIABLE*/
			Error(12); i=0;
		  }
        GetSym();
		// if (SYM==BECOMES) GetSym();
		// else Error(13);
		// EXPRESSION(FSYS,LEV,TX);
		// if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
		
		// ↓↓↓ 新增部分 ↓↓↓
    if (SYM==BECOMES) {
        GetSym();
        EXPRESSION(FSYS,LEV,TX);
        if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    }
    else if(SYM==TIMESBECOMES) {    // *= 逻辑运算
        GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
        EXPRESSION(FSYS,LEV,TX);
        GEN(OPR,0,4);
        if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    }
    else if(SYM==SLASHBECOMES) {    // /= 逻辑运算
        GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
        EXPRESSION(FSYS,LEV,TX);
        GEN(OPR,0,5);
        if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    }
    else if(SYM==PLUSBECOMES) {    // += 逻辑运算
        GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
        EXPRESSION(FSYS,LEV,TX);
        GEN(OPR,0,2);
        if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    }
    else if(SYM==MINUSBECOMES) {    // -= 逻辑运算
        GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
        EXPRESSION(FSYS,LEV,TX);
        GEN(OPR,0,3);
        if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    }
	else if(SYM==INCSYM) {          // i++ 逻辑运算
        if(i!=0)
            GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GEN(LIT,0,1);
        GEN(OPR,0,2);
        if(i!=0)
            GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
    }
    else if(SYM==DECSYM) {          // i-- 逻辑运算
        if(i!=0)
            GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GEN(LIT,0,1);
        GEN(OPR,0,3);
        if(i!=0)
            GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
    }
    else Error(13);
    // ↑↑↑ 新增部分 ↑↑↑
		
		break;
	case READSYM:
		GetSym();
		if (SYM!=LPAREN) Error(34);
		else
		  do {
			GetSym();
			if (SYM==IDENT) i=POSITION(ID,TX);
			else i=0;
			if (i==0) Error(35);
			else {
			  GEN(OPR,0,16);
			  GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
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
			GetSym();
			EXPRESSION(SymSetUnion(SymSetNew(RPAREN,COMMA),FSYS),LEV,TX);
			GEN(OPR,0,14);
		  }while(SYM==COMMA);
		  if (SYM!=RPAREN) Error(33);
		  else GetSym();
		}
		GEN(OPR,0,15);
		break; /*WRITESYM*/
	case CALLSYM:
		GetSym();
		if (SYM!=IDENT) Error(14);
		else {
		  i=POSITION(ID,TX);
		  if (i==0) Error(11);
		  else
			if (TABLE[i].KIND==PROCEDUR)
			  GEN(CAL,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
			else Error(15);
		  GetSym();
		}
		break;
	case IFSYM:
		GetSym();
		CONDITION(SymSetUnion(SymSetNew(THENSYM,DOSYM),FSYS),LEV,TX);
		if (SYM==THENSYM) GetSym();
		else Error(16);
		CX1=CX;  GEN(JPC,0,0);		
		//STATEMENT(FSYS,LEV,TX);  CODE[CX1].A=CX; 注释掉
		
        // ↓↓↓ 新增部分 ↓↓↓	
		STATEMENT(SymSetUnion(SymSetNew(ELSESYM),FSYS),LEV,TX);		
        if(SYM!=ELSESYM)
            CODE[CX1].A=CX;
        else {
            GetSym();
            CX2=CX;
            GEN(JMP,0,0);		//直接跳转，执行完Then后面的则跳转到条件语句最后面
            CODE[CX1].A=CX;		//回填条件跳转，填回else语句块中第一句
            STATEMENT(FSYS,LEV,TX);
            CODE[CX2].A=CX;  	//回填直接跳转地址
        }
        // ↑↑↑ 新增部分 ↑↑↑
		
		break;
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
		
	// ↓↓↓ 新增部分 ↓↓↓
// 扩充语句。FOR <变量>:=<表达式>STEP<表达式>UNTIL<表达式>Do<语句> ；
case FORSYM:
    GetSym();
    if(SYM!=IDENT) Error(47);
    else i=POSITION(ID,TX);
    if(i==0) Error(11);
    else if(TABLE[i].KIND!=VARIABLE){
        Error(12); i=0;
    }
    GetSym();
    if(SYM=BECOMES) GetSym();
    else Error(13);
    
    EXPRESSION(SymSetUnion(SymSetNew(STEPSYM),FSYS),LEV,TX);
    if(i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    if(SYM==STEPSYM) GetSym();
    else Error(46);
    CX1=CX;
    GEN(JMP,0,0);
    CX3=CX;
    
    EXPRESSION(SymSetUnion(SymSetNew(UNTILSYM),FSYS),LEV,TX);
    GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    GEN(OPR,0,2);
    GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    if(SYM==UNTILSYM) GetSym();
    else Error(47);
    CODE[CX1].A=CX;
    
    EXPRESSION(SymSetUnion(SymSetNew(DOSYM),FSYS),LEV,TX);
    GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
    GEN(OPR,0,11);
    CX2=CX;
    GEN(JPC,0,0);
    if(SYM==DOSYM) GetSym();
    else Error(48);
    
    STATEMENT(FSYS,LEV,TX);
    GEN(JMP,0,CX3);
    CODE[CX2].A=CX;
    break;
	
	// 处理 ++i
case INCSYM:   
    GetSym();
    if(SYM==IDENT){
        i=POSITION(ID,TX);
        if(i==0) Error(11);
        else if(TABLE[i].KIND!=VARIABLE){
            Error(12);
            i=0;
        }
        if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GEN(LIT,0,1);
        GEN(OPR,0,2);
        if(i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
    }
    else Error(45);
    break;
// 处理 --i
case DECSYM: 
    GetSym();
    if(SYM==IDENT){
        i=POSITION(ID,TX);
        if(i==0) Error(11);
        else if(TABLE[i].KIND!=VARIABLE){
            Error(12);
            i=0;
        }
        if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GEN(LIT,0,1);
        GEN(OPR,0,3);
        if(i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
        GetSym();
    }
    else Error(45);
    break;
// ↑↑↑ 新增部分 ↑↑↑
  }
  TEST(FSYS,SymSetNULL(),19);
} /*STATEMENT*/
//---------------------------------------------------------------------------
void Block(int LEV, int TX, SYMSET FSYS) {
  int DX=3;    /*DATA ALLOCATION INDEX*/
  int TX0=TX;  /*INITIAL TABLE INDEX*/
  int CX0=CX;  /*INITIAL CODE INDEX*/
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
    if (SYM==VARSYM) {
      GetSym();
      do {
        VarDeclaration(LEV,TX,DX);
        while (SYM==COMMA) { GetSym(); VarDeclaration(LEV,TX,DX); }
	    if (SYM==SEMICOLON) GetSym();
	    else Error(5);
      }while(SYM==IDENT);
    }
    while ( SYM==PROCSYM) {
      GetSym();
	  if (SYM==IDENT) { ENTER(PROCEDUR,LEV,TX,DX); GetSym(); }
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
    TEST(SymSetAdd(IDENT,STATBEGSYS), DECLBEGSYS,7);
  }while(SymIn(SYM,DECLBEGSYS));
  CODE[TABLE[TX0].vp.ADR].A=CX;
  TABLE[TX0].vp.ADR=CX;   /*START ADDR OF CODE*/
  TABLE[TX0].vp.SIZE=DX;  /*SIZE OF DATA SEGMENT*/
  GEN(INI,0,DX);
  STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX);
  GEN(OPR,0,0);  /*RETURN*/
  TEST(FSYS,SymSetNULL(),8);
  ListCode(CX0);
} /*Block*/
//---------------------------------------------------------------------------
int BASE(int L,int B,int S[]) {
  int B1=B; /*FIND BASE L LEVELS DOWN*/
  while (L>0) { B1=S[B1]; L=L-1; }
  return B1;
} /*BASE*/
//---------------------------------------------------------------------------
void Interpret() {
  const STACKSIZE = 500;
  int P,B,T; 		/*PROGRAM BASE TOPSTACK REGISTERS*/
  INSTRUCTION I;
  int S[STACKSIZE];  	/*DATASTORE*/
  Form1->printfs("~~~ RUN PL0 ~~~");
  fprintf(FOUT,"~~~ RUN PL0 ~~~\n");
  T=0; B=1; P=0;
  S[1]=0; S[2]=0; S[3]=0;
  do {
    I=CODE[P]; P=P+1;
    switch (I.F) {
      case LIT: T++; S[T]=I.A; break;
	  case OPR:
	    switch (I.A) { /*OPERATOR*/
	      case 0: /*RETURN*/ T=B-1; P=S[T+3]; B=S[T+2]; break;
	      case 1: S[T]=-S[T];  break;
	      case 2: T--; S[T]=S[T]+S[T+1];   break;
	      case 3: T--; S[T]=S[T]-S[T+1];   break;
	      case 4: T--; S[T]=S[T]*S[T+1];   break;
	      //case 5: T--; S[T]=S[T] % S[T+1]; break;
		  case 5: T--; S[T]=S[T] / S[T+1]; break;
	      case 6: S[T]=(S[T]%2!=0);        break;
	      case 8: T--; S[T]=S[T]==S[T+1];  break;
	      case 9: T--; S[T]=S[T]!=S[T+1];  break;
	      case 10: T--; S[T]=S[T]<S[T+1];   break;
	      case 11: T--; S[T]=S[T]>=S[T+1];  break;
	      case 12: T--; S[T]=S[T]>S[T+1];   break;
	      case 13: T--; S[T]=S[T]<=S[T+1];  break;
	      case 14: Form1->printls("",S[T]); fprintf(FOUT,"%d\n",S[T]); T--;
                   break;
	      case 15: /*Form1->printfs(""); fprintf(FOUT,"\n"); */ break;
	      case 16: T++;  S[T]=InputBox("输入","请键盘输入：", 0).ToInt();
                   Form1->printls("? ",S[T]); fprintf(FOUT,"? %d\n",S[T]);
		           break;
	    }
	    break;
      case LOD: T++; S[T]=S[BASE(I.L,B,S)+I.A]; break;
      case STO: S[BASE(I.L,B,S)+I.A]=S[T]; T--; break;
	  case CAL: /*GENERAT NEW Block MARK*/
	      S[T+1]=BASE(I.L,B,S); S[T+2]=B; S[T+3]=P;
	      B=T+1; P=I.A; break;
	  case INI: T=T+I.A;  break;
	  case JMP: P=I.A; break;
      case JPC: if (S[T]==0) P=I.A;  T--;  break;
    } /*switch*/
  }while(P!=0);
  Form1->printfs("~~~ END PL0 ~~~");
  fprintf(FOUT,"~~~ END PL0 ~~~\n");
} /*Interpret*/
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonRunClick(TObject *Sender) {
  for (CH=' '; CH<='^'; CH++) SSYM[CH]=NUL;
  // strcpy(KWORD[ 1],"BEGIN");    strcpy(KWORD[ 2],"CALL");
  // strcpy(KWORD[ 3],"CONST");    strcpy(KWORD[ 4],"DO");
  // strcpy(KWORD[ 5],"END");      strcpy(KWORD[ 6],"IF");
  // strcpy(KWORD[ 7],"ODD");      strcpy(KWORD[ 8],"PROCEDURE");
  // strcpy(KWORD[ 9],"PROGRAM");  strcpy(KWORD[10],"READ");
  // strcpy(KWORD[11],"THEN");     strcpy(KWORD[12],"VAR");
  // strcpy(KWORD[13],"WHILE");    strcpy(KWORD[14],"WRITE");
  // WSYM[ 1]=BEGINSYM;   WSYM[ 2]=CALLSYM;
  // WSYM[ 3]=CONSTSYM;   WSYM[ 4]=DOSYM;
  // WSYM[ 5]=ENDSYM;     WSYM[ 6]=IFSYM;
  // WSYM[ 7]=ODDSYM;     WSYM[ 8]=PROCSYM;
  // WSYM[ 9]=PROGSYM;    WSYM[10]=READSYM;
  // WSYM[11]=THENSYM;    WSYM[12]=VARSYM;
  // WSYM[13]=WHILESYM;   WSYM[14]=WRITESYM;
  // SSYM['+']=PLUS;      SSYM['-']=MINUS;
  // SSYM['*']=TIMES;     SSYM['/']=SLASH;
  // SSYM['(']=LPAREN;    SSYM[')']=RPAREN;
  // SSYM['=']=EQL;       SSYM[',']=COMMA;
  // SSYM['.']=PERIOD;    SSYM['#']=NEQ;
  // SSYM[';']=SEMICOLON;
  strcpy(MNEMONIC[LIT],"LIT");   strcpy(MNEMONIC[OPR],"OPR");
  strcpy(MNEMONIC[LOD],"LOD");   strcpy(MNEMONIC[STO],"STO");
  strcpy(MNEMONIC[CAL],"CAL");   strcpy(MNEMONIC[INI],"INI");
  strcpy(MNEMONIC[JMP],"JMP");   strcpy(MNEMONIC[JPC],"JPC");
  
  // 新增5个保留字。ELSE, FOR, STEP, UNTIL, RETURN
  strcpy(KWORD[ 1],"BEGIN");
  strcpy(KWORD[ 2],"CALL");
  strcpy(KWORD[ 3],"CONST");
  strcpy(KWORD[ 4],"DO");
  strcpy(KWORD[ 5],"ELSE");     // 增加保留字1。 ELSE
  strcpy(KWORD[ 6],"END");
  strcpy(KWORD[ 7],"FOR");      // 增加保留字2。 FOR
  strcpy(KWORD[ 8],"IF");
  strcpy(KWORD[ 9],"ODD");
  strcpy(KWORD[10],"PROCEDURE");
  strcpy(KWORD[11],"PROGRAM");
  strcpy(KWORD[12],"READ");
  strcpy(KWORD[13],"RETURN");   // 增加保留字5。 RETURN
  strcpy(KWORD[14],"STEP");		// 增加保留字3。 STEP
  strcpy(KWORD[15],"THEN");
  strcpy(KWORD[16],"UNTIL");	// 增加保留字4。 UNTIL
  strcpy(KWORD[17],"VAR");
  strcpy(KWORD[18],"WHILE");
  strcpy(KWORD[19],"WRITE");
  
  // 新增5个保留字符号。ELSESYM, FORSYM, STEPSYM, UNTILSYM, RETURNSYM
  WSYM[ 1]=BEGINSYM;
  WSYM[ 2]=CALLSYM;
  WSYM[ 3]=CONSTSYM;
  WSYM[ 4]=DOSYM;
  WSYM[ 5]=ELSESYM;		// 增加保留字符号1。 ELSESYM
  WSYM[ 6]=ENDSYM;
  WSYM[ 7]=FORSYM;      // 增加保留字符号2。 FORSYM
  WSYM[ 8]=IFSYM;
  WSYM[ 9]=ODDSYM;
  WSYM[10]=PROCSYM;
  WSYM[11]=PROGSYM;
  WSYM[12]=READSYM;
  WSYM[13]=RETURNSYM;   // 增加保留字符号5。 RETURNSYM
  WSYM[14]=STEPSYM;     // 增加保留字符号3。 STEPSYM
  WSYM[15]=THENSYM;
  WSYM[16]=UNTILSYM;    // 增加保留字符号4。 UNTILSYM
  WSYM[17]=VARSYM;
  WSYM[18]=WHILESYM;
  WSYM[19]=WRITESYM;
  
  // 新增2个符号。 & ！
  SSYM['+']=PLUS;      SSYM['-']=MINUS;
  SSYM['*']=TIMES;     SSYM['/']=SLASH;
  SSYM['(']=LPAREN;    SSYM[')']=RPAREN;
  SSYM['=']=EQL;       SSYM[',']=COMMA;
  SSYM['.']=PERIOD;    // SSYM['#']=NEQ;	注释掉
  SSYM[';']=SEMICOLON;
  SSYM['&']=ANDSYM;     // 新增符号。 &
  SSYM['!']=NOTSYM;     // 新增符号。 !

  DECLBEGSYS=(int*)malloc(sizeof(int)*SYMNUM);
  STATBEGSYS=(int*)malloc(sizeof(int)*SYMNUM);
  FACBEGSYS =(int*)malloc(sizeof(int)*SYMNUM);
  for(int j=0; j<SYMNUM; j++) {
	DECLBEGSYS[j]=0;  STATBEGSYS[j]=0;  FACBEGSYS[j] =0;
  }
  DECLBEGSYS[CONSTSYM]=1;
  DECLBEGSYS[VARSYM]=1;
  DECLBEGSYS[PROCSYM]=1;
  STATBEGSYS[BEGINSYM]=1;
  STATBEGSYS[CALLSYM]=1;
  STATBEGSYS[IFSYM]=1;
  STATBEGSYS[WHILESYM]=1;
  STATBEGSYS[WRITESYM]=1;
  FACBEGSYS[IDENT] =1;
  FACBEGSYS[NUMBER]=1;
  FACBEGSYS[LPAREN]=1;

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

