package net.devyy;

import java.util.BitSet;

public class SymSet extends BitSet {
	/**
	 * 这个域没有特别意义 
	 */
	private static final long serialVersionUID = 8136959240158320958L;

	/**
	 * 构造一个符号集合
	 * @param nbits 这个集合的容量
	 */
	public SymSet(int nbits) {
		super(nbits);
	}

	/**
	 * 把一个符号放到集合中
	 * @param s 要放置的符号
	 */
	public void set(Symbol s) {
		set(s.ordinal());
	}
	
	/**
	 * 检查一个符号是否在集合中
	 * @param s 要检查的符号
	 * @return 若符号在集合中，则返回true，否则返回false
	 */
	public boolean get(Symbol s) {
		return get(s.ordinal());
	}
}
