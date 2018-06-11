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

### 指令表

| 指令 | 说明 |
|:-:| - |
| CAL L A | 调用地址为 A 的过程（置指令地址寄存器为A）；L 为调用过程与被调用过程的层差；设置被调用过程的3 个联系单元 |
| INT 0 A | 在栈顶开辟 A 个存储单元，服务于被调用的过程;A 等于该过程的局部变量数加 3;3 个特殊的联系单元 |
| JMP 0 A | 无条件转移至地址 A，即置指令地址寄存器为A |
| JPC 0 A | 条件转移指令；若栈顶为 0，则转移至地址 A，即置指令地址寄存器为A ；T 减1 |
| LIT 0 A | 立即数存入栈顶，即置T 所指存储单元的值为Al； T 加 1 |
| LOD L A | 将层差为L、偏移量为A的存储单元的值取到栈顶； T 加 1 |
| OPR 0 0 | 过程调用结束后,返回调用点并退栈； 重置基址寄存器和栈顶寄存器 |
| OPR 0 1 | 求栈顶元素的相反数，结果值留在栈顶 |
| OPR 0 2 | 次栈顶与栈顶的值相加，结果存入次栈顶； T 减 1 |
| OPR 0 3 | 次栈顶的值减去栈顶的值，结果存入次栈顶； T 减 1 |
| OPR 0 4 | 次栈顶的值乘以栈顶的值，结果存入次栈顶； T 减 1 |
| OPR 0 5 | 次栈顶的值除以栈顶的值，结果存入次栈顶； T 减 1 |
| OPR 0 6 | 栈顶元素的奇偶判断，若为奇数，结果为1；若为偶数，结果为0 ；结果值留在栈顶 |
| OPR 0 8 | 比较次栈顶与栈顶是否相等；若相等，结果为0；存结果至次栈顶；T 减1 |
| OPR 0 9 | 比较次栈顶与栈顶是否不相等；若不相等，结果为0；存结果至次栈顶；T 减1 |
| OPR 0 10 | 比较次栈顶是否小于栈顶； 若小于，结果为0；存结果至次栈顶；T 减1 |
| OPR 0 11 | 比较次栈顶是否大于等于栈顶； 若大于等于，结果为0；存结果至次栈顶；T 减1 |
| OPR 0 12 | 比较次栈顶是否大于栈顶； 若大于，结果为0；存结果至次栈顶；T 减1 |
| OPR 0 13 | 比较次栈顶是否小于等于栈顶； 若小于等于，结果为0；存结果至次栈顶；T 减1 |
| OPR 0 14 | 栈顶的值输出至控制台屏幕； T  减 1 |
| OPR 0 15 | 控制台屏幕输出一个换行 |
| OPR 0 16 | 从控制台读入一行输入，置入栈顶； T 加 1 |
| STO L A | T 减 1； 将栈顶的值存入层差为L、偏移量为A的存储单元 |



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

## 

修改前：

	case IDENT:
			i=POSITION(ID,TX);
			if (i==0) Error(11);
			else
			  if (TABLE[i].KIND!=VARIABLE) { /*ASSIGNMENT TO NON-VARIABLE*/
				Error(12); i=0;
			  }
	        GetSym();
			if (SYM==BECOMES) GetSym();
			else Error(13);
			EXPRESSION(FSYS,LEV,TX);
			if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
			break;

修改后：


修改前：

	// ↓↓↓ 新增部分 ↓↓↓		
	// 用来检验保留字是否添加成功的标志
	case FORSYM:
        GetSym();
        Form1->printfs("保留字：FORSYM~~~~");
        break;
    case STEPSYM:
        GetSym();
        Form1->printfs("保留字：STEPSYM~~~~");
        break;
    case UNTILSYM:
        GetSym();
        Form1->printfs("保留字：UNTILSYM~~~~");
        break;
    case RETURNSYM:
        GetSym();
        Form1->printfs("保留字：RETURNSYM~~~~");
        break;
    case DOSYM:
        GetSym();
        Form1->printfs("保留字：DOSYM~~~~");
        break;
		
	// 用来检验运算符是否添加成功的标志。
    case TIMESBECOMES:
        GetSym();
        Form1->printfs("运算符：*= ~~~~");
        break;
    case SLASHBECOMES:
        GetSym();
        Form1->printfs("运算符：/= ~~~~");
        break;
    case ANDSYM:
        GetSym();
        Form1->printfs("运算符：&  ~~~~");
        break;
    case ORSYM:
        GetSym();
        Form1->printfs("运算符：|| ~~~~");
        break;
    case NOTSYM:
        GetSym();
        Form1->printfs("运算符：!  ~~~~");
        break;
	// ↑↑↑ 新增部分 ↑↑↑	

修改后：

