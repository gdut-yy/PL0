package net.devyy;

/**
 * 符号类型，为避免和Java的关键字Object冲突，我们改成Objekt
 */
enum Objekt {
	constant, variable, procedure
}

/**
 *　　这个类封装了PL/0编译器的符号表，C语言版本中关键的全局变量tx和table[]就在这里。
 */
public class Table {
	/**
	 *　　即C语言版本中的tablestruct结构。
	 */
	public class Item {
		String name;		// 名字
		Objekt kind;		// 类型：const, var or procedure
		int val;			// 数值，仅const使用
		int level;			// 所处层，var和procedure使用
		int adr;			// 地址，var和procedure使用
		int size;			// 需要分配的数据区空间, 仅procedure使用
	}
	
	/**
	 * 名字表，请使用get()函数访问
	 * @see #get(int)
	 */
	private Item[] table = new Item[PL0.txmax];
	
	/**
	 * 当前名字表项指针，也可以理解为当前有效的名字表大小（table size）
	 */
	public int tx = 0;
	
	/**
	 * 获得名字表某一项的内容
	 * @param i 名字表中的位置
	 * @return 名字表第 i 项的内容
	 */
	public Item get(int i) {
		if (table[i] == null) {
			table[i] = new Item();
			table[i].name = "";
		}
		return table[i];
	}
	
	/**
	 * 把某个符号登陆到名字表中，注意参数跟C语言版本不同
	 * @param k   该符号的类型：const, var, procedure
	 * @param lev 名字所在的层次
	 * @param dx  当前应分配的变量的相对地址，注意调用enter()后dx要加一
	 */
	public void enter(Objekt k, int lev, int dx) {
		tx ++;
		Item item = get(tx);
		item.name = PL0.lex.id;			// 注意id和num都是从词法分析器获得
		item.kind = k;
		switch (k) {
		case constant:					// 常量名字
			if (PL0.lex.num > PL0.amax) {
				Err.report(31);		// 数字过大溢出
				item.val = 0;
			} else {
				item.val = PL0.lex.num;
			}
			break;
		case variable:					// 变量名字 
			item.level = lev;
			item.adr = dx;
			break;
		case procedure:					// 过程名字
			item.level = lev;
			break;
		}
	}
	
	/**
	 * 打印符号表内容，摘自C语言版本的 block() 函数。
	 * @param start 当前作用域符号表区间的左端
	 */
	public void debugTable(int start) {
		if (!PL0.tableswitch)
			return;
		System.out.println("TABLE:");
		if (start >= tx)
			System.out.println("    NULL");
		for (int i=start+1; i<= tx; i++) {
			String msg = "OOPS! UNKNOWN TABLE ITEM!";
			switch (table[i].kind) {
			case constant:
				msg = "    " + i + " const " + table[i].name + " val=" + table[i].val;
				break;
			case variable:
				msg = "    " + i + " var   " + table[i].name + " lev=" + table[i].level + " addr=" + table[i].adr;
				break;
			case procedure:
				msg = "    " + i + " proc  " + table[i].name + " lev=" + table[i].level + " addr=" + table[i].adr + " size=" + table[i].size;
				break;
			}
			System.out.println(msg);
			PL0.fas.println(msg);
		}
		System.out.println();
	}

	/**
	 * 在名字表中查找某个名字的位置
	 * @param idt 要查找的名字
	 * @return 如果找到则返回名字项的下标，否则返回0
	 */
	public int position(String idt) {
		for (int i = tx; i > 0; i--)
			if (get(i).name.equals(idt))
				return i;
		
		return 0;
	}
}
