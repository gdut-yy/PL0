# PL/0 编译器的扩充--课程设计

## 一、实验目的与要求

	基本内容（成绩范围：“中”、“及格”或“不及格”）
	（1）扩充赋值运算：*= 和 /=
	（2）扩充语句（Pascal的FOR语句）:
	 FOR <变量>:=<表达式>STEP<表达式UNTIL<表达式>Do<语句>
	
	选做内容（成绩评定范围扩大到：“优”和“良”）
	（1）增加类型：① 字符类型；  ② 实数类型。
	（2）扩充运算：++ 和 --（要求作为表达式实现） 
	（3）扩充函数：① 有返回值和返回语句；② 有参数函数。   
	（4）增加一维数组类型（可增加指令）。   
	（5）其他典型语言设施。

## 二、实验环境与工具 

	1、源语言：PL/0语言，PL/0语言是PASCAL语言的子集，它的编译程序是一个编译解析执行系统，后缀名为.PL0；
	2、目标语言：生成文件后缀为*.COD的目标代码 
	3、实现平台：Borland C++Builder 6 
	4、运行平台：Windows 7 64位 

## 实现内容

## 结构设计说明

## 主要成分描述


## 测试用例

## 开发过程和完成情况



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