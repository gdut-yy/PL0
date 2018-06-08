package net.devyy;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;

/**
 *<p>这个版本的 PL/0 编译器根据 C 语言的版本改写而成。两个版本在基本逻辑上是一致
 *的，有些地方可能有所改动，例如getsym()和statement()两个函数，另外请注意C语言
 *版本中的全局变量分散到构成编译器各个类中，为便于查找，保留了这些全局变量原来的名字。</p>
 *
 *<p>阅读过程中若有疑问请及时咨询你的助教。</p>
 */
public class PL0 {
	// 编译程序的常数
	public static final int al = 10;			// 符号的最大长度
	public static final int amax = 2047;		// 最大允许的数值
	public static final int cxmax = 500;		// 最多的虚拟机代码数
	public static final int levmax = 3;			// 最大允许过程嵌套声明层数 [0, levmax]
	public static final int nmax = 14;			// number的最大位数
	public static final int norw = 32;			// 关键字个数
	public static final int txmax = 100;		// 名字表容量
	
	// 一些全局变量，其他关键的变量分布如下：
	// cx, code : Interpreter
	// dx : Parser
	// tx, table : Table
	public static PrintStream fa;				// 输出虚拟机代码
	public static PrintStream fa1;				// 输出源文件及其各行对应的首地址
	public static PrintStream fa2;				// 输出结果
	public static PrintStream fas;				// 输出名字表
	public static boolean listswitch;			// 显示虚拟机代码与否
	public static boolean tableswitch;			// 显示名字表与否
	
	// 一个典型的编译器的组成部分
	public static Scanner lex;					// 词法分析器
	public static Parser  parser;				// 语法分析器
	public static Interpreter interp;			// 类P-Code解释器（及目标代码生成工具）
	public static Table table;					// 名字表
	
	// 为避免多次创建BufferedReader，我们使用全局统一的Reader
	public static BufferedReader stdin;			// 标准输入
	
	/**
	 * 构造函数，初始化编译器所有组成部分
	 * @param fin PL/0 源文件的输入流
	 */
	public PL0(BufferedReader fin) {
		// 各部件的构造函数中都含有C语言版本的 init() 函数的一部分代码
		table = new Table();
		interp = new Interpreter();
		lex = new Scanner(fin);
		parser = new Parser(lex, table, interp);
	}

	/**
	 * 执行编译动作
	 * @return 是否编译成功
	 */
	boolean compile() {
		boolean abort = false;
		
		try {
			PL0.fa = new PrintStream("fa.tmp");
			PL0.fas = new PrintStream("fas.tmp");
			parser.nextSym();		// 前瞻分析需要预先读入一个符号
			parser.parse();			// 开始语法分析过程（连同语法检查、目标代码生成）
		} catch (Error e) {
			// 如果是发生严重错误则直接中止
			abort = true;
		} catch (IOException e) {
		} finally { 
			PL0.fa.close();
			PL0.fa1.close();
			PL0.fas.close();
		}
		if (abort)
			System.exit(0);
				
		// 编译成功是指完成编译过程并且没有错误
		return (Err.err == 0);
	}

	/**
	 * 主函数
	 */
	public static void main(String[] args) {
		// 原来 C 语言版的一些语句划分到compile()和Parser.parse()中
		String fname = "";
		stdin = new BufferedReader(new InputStreamReader(System.in));
		BufferedReader fin;
		try {
			// 输入文件名
			fname = "";
			System.out.print("Input pl/0 file?   ");
			while (fname.equals(""))
				fname = stdin.readLine();
			fin = new BufferedReader(new FileReader(fname), 4096);

			// 是否输出虚拟机代码
			fname = "";
			System.out.print("List object code?(Y/N)");
			while (fname.equals(""))
				fname = stdin.readLine();
			PL0.listswitch = (fname.charAt(0)=='y' || fname.charAt(0)=='Y');
			
			// 是否输出名字表
			fname = "";
			System.out.print("List symbol table?(Y/N)");
			while (fname.equals(""))
				fname = stdin.readLine();
			PL0.tableswitch = (fname.charAt(0)=='y' || fname.charAt(0)=='Y');
			
			PL0.fa1 = new PrintStream("fa1.tmp");
			PL0.fa1.println("Input pl/0 file?   " + fname);

			// 构造编译器并初始化
			PL0 pl0 = new PL0(fin);
			
			if (pl0.compile()) {
				// 如果成功编译则接着解释运行
				PL0.fa2 = new PrintStream("fa2.tmp");
				interp.interpret();
				PL0.fa2.close();
			} else {
				System.out.print("Errors in pl/0 program");
			}
			
		} catch (IOException e) {
			System.out.println("Can't open file!");
		}

		System.out.println();
	}
}
