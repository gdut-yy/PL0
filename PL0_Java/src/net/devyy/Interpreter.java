package net.devyy;

/**
 * 类P-Code指令类型
 */
enum Fct {
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC
}

/**
 *　　这个类对应C语言版本中的 fct 枚举类型和 instruction 结构，代表虚拟机指令
 */
class Instruction {
	/**
	 * 虚拟机代码指令
	 */
	public Fct f;
	
	/**
	 * 引用层与声明层的层次差
	 */
	public int l;
	
	/**
	 * 指令参数
	 */
	public int a;
}

/**
 *　　类P-Code代码解释器（含代码生成函数），这个类包含了C语言版中两个重要的全局变量 cx 和 code
 */
public class Interpreter {
	// 解释执行时使用的栈大小
	final int stacksize = 500;
	
	/**
	 * 虚拟机代码指针，取值范围[0, cxmax-1] 
	 */
	public int cx = 0;
	
	/**
	 * 存放虚拟机代码的数组
	 */
	public Instruction[] code = new Instruction[PL0.cxmax];
	
	/**
	 * 生成虚拟机代码
	 * @param x instruction.f
	 * @param y instruction.l
	 * @param z instruction.a
	 */
	public void gen(Fct x, int y, int z) {
		if (cx >= PL0.cxmax) {
			throw new Error("Program too long");
		}
		
		code[cx] = new Instruction();
		code[cx].f = x;
		code[cx].l = y;
		code[cx].a = z;
		cx ++;
	}

	/**
	 * 输出目标代码清单
	 * @param start 开始输出的位置
	 */
	public void listcode(int start) {
		if (PL0.listswitch) {
			for (int i=start; i<cx; i++) {
				String msg = i + " " + code[i].f + " " + code[i].l + " " + code[i].a;
				System.out.println(msg);
				PL0.fa.println(msg);
			}
		}
	}
	
	/**
	 * 解释程序
	 */
	public void interpret() {
		int p, b, t;						// 指令指针，指令基址，栈顶指针
		Instruction i;							// 存放当前指令
		int[] s = new int[stacksize];		// 栈
		
		System.out.println("start pl0");
		t = b = p = 0;
		s[0] = s[1] = s[2] = 0;
		do {
			i = code[p];					// 读当前指令
			p ++;
			switch (i.f) {
			case LIT:				// 将a的值取到栈顶
				s[t] = i.a;
				t++;
				break;
			case OPR:				// 数学、逻辑运算
				switch (i.a)
				{
				case 0:
					t = b;
					p = s[t+2];
					b = s[t+1];
					break;
				case 1:
					s[t-1] = -s[t-1];
					break;
				case 2:
					t--;
					s[t-1] = s[t-1]+s[t];
					break;
				case 3:
					t--;
					s[t-1] = s[t-1]-s[t];
					break;
				case 4:
					t--;
					s[t-1] = s[t-1]*s[t];
					break;
				case 5:
					t--;
					s[t-1] = s[t-1]/s[t];
					break;
				case 6:
					s[t-1] = s[t-1]%2;
					break;
				case 8:
					t--;
					s[t-1] = (s[t-1] == s[t] ? 1 : 0);
					break;
				case 9:
					t--;
					s[t-1] = (s[t-1] != s[t] ? 1 : 0);
					break;
				case 10:
					t--;
					s[t-1] = (s[t-1] < s[t] ? 1 : 0);
					break;
				case 11:
					t--;
					s[t-1] = (s[t-1] >= s[t] ? 1 : 0);
					break;
				case 12:
					t--;
					s[t-1] = (s[t-1] > s[t] ? 1 : 0);
					break;
				case 13:
					t--;
					s[t-1] = (s[t-1] <= s[t] ? 1 : 0);
					break;
				case 14:
					System.out.print(s[t-1]);
					PL0.fa2.print(s[t-1]);
					t--;
					break;
				case 15:
					System.out.println();
					PL0.fa2.println();
					break;
				case 16:
					System.out.print("?");
					PL0.fa2.print("?");
					s[t] = 0;
					try {
						s[t] = Integer.parseInt(PL0.stdin.readLine());
					} catch (Exception e) {}
					PL0.fa2.println(s[t]);
					t++;
					break;
				}
				break;
			case LOD:				// 取相对当前过程的数据基地址为a的内存的值到栈顶
				s[t] = s[base(i.l,s,b)+i.a];
				t++;
				break;
			case STO:				// 栈顶的值存到相对当前过程的数据基地址为a的内存
				t--;
				s[base(i.l, s, b) + i.a] = s[t];
				break;
			case CAL:				// 调用子过程
				s[t] = base(i.l, s, b); 	// 将静态作用域基地址入栈
				s[t+1] = b;					// 将动态作用域基地址入栈
				s[t+2] = p;					// 将当前指令指针入栈
				b = t;  					// 改变基地址指针值为新过程的基地址
				p = i.a;   					// 跳转
				break;
			case INT:			// 分配内存
				t += i.a;
				break;
			case JMP:				// 直接跳转
				p = i.a;
				break;
			case JPC:				// 条件跳转（当栈顶为0的时候跳转）
				t--;
				if (s[t] == 0)
					p = i.a;
				break;
			}
		} while (p != 0);
	}
	
	/**
	 * 通过给定的层次差来获得该层的堆栈帧基地址
	 * @param l 目标层次与当前层次的层次差
	 * @param s 运行栈
	 * @param b 当前层堆栈帧基地址
	 * @return 目标层次的堆栈帧基地址
	 */
	private int base(int l, int[] s, int b) {
		int b1 = b;
		while (l > 0) {
			b1 = s[b1];
			l --;
		}
		return b1;
	}
}
