package net.devyy;

/**
 *　　这个类只是包含了报错函数以及错误计数器。
 */
public class Err {
	/**
	 * 错误计数器，编译过程中一共有多少个错误
	 */
	public static int err = 0;
	
	/**
	 * 报错函数
	 * @param errcode 错误码
	 */
	public static void report(int errcode) {
		char[] s = new char[PL0.lex.cc-1];
		java.util.Arrays.fill(s, ' ');
		String space = new String(s);
		System.out.println("****" + space + "!" + errcode);
		PL0.fa1.println("****" + space + "!" + errcode);
		err ++;
	}
}
