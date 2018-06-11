# PL/0 编译器 原始代码

本项目仅作为对照组，不进行后续修改更新。

源代码中有一个bug，有可能是故意留的（Unit1.cpp 文件中 line 555）

	case 5: T--; S[T]=S[T] % S[T+1]; break;

应为：

	case 5: T--; S[T]=S[T] / S[T+1]; break;

## E01.PL0 文件：

	PROGRAM EX01;
	VAR A,B,C;
	BEGIN
	  A:=88;
	  READ(B);
	  C=A+B*(3+B);
	  WRITE(C)
	END.

## E0101.PL0 文件：

	PROGRAM EX01;
	VAR A,B;
	BEGIN
	  A:=88;
	  B:=11;
	  WRITE(A);
	END.

## P9101.PL0 文件：

	PROGRAM MAIN;
	CONST A=10;
	VAR B,C;
	PROCEDURE P;
	BEGIN END;
	PROCEDURE S;
	BEGIN END;
	BEGIN
	  B:=8;
	  READ(C);
	  WHILE C>0 DO
	    BEGIN
	      WRITE(B);
	      CALL S;
	    END;
	  CALL P;
	END.

## P9102.PL0 文件：

	PROGRAM MAIN;
	VAR A,B,C;
	BEGIN
	  A:=100;
	  B:=200;
	  C:=300;
	  A:=C+(B-A);
	END.

## P9104.PL0 文件：

	PROGRAM MAIN;
	CONST A=10;
	VAR B,C;
	PROCEDURE P;
	VAR D;
	 PROCEDURE Q;
	 CONST E=5; VAR F;
	  PROCEDURE R;
	  VAR G;
	  BEGIN G:=A+E; F:=G;
	    WRITE(G)
	  END;
	 BEGIN CALL R; WRITE(F);
	   F:=A+E*F; WRITE(F,D)
	 END;
	BEGIN D:=A+B; WRITE(D); CALL Q END;
	PROCEDURE S;
	BEGIN CALL P; B:=-10; WRITE(B) END;
	BEGIN
	  B:=8;
	  READ(C);
	  WHILE C>0 DO
	    BEGIN
	      WRITE(B); CALL S;
	      B:=B+C; WRITE(B);
	      READ(C)
	    END;
	  CALL P
	END.

## T1.PL0 文件：

	PROGRAM P1;
	CONST I=5, B=20; C=1, D=2;
	VAR I; X,Y;
	PROCEDURE J;
	BEGIN
	  X:=B
	END;
	PROCEDURE K;
	BEGIN
	  Y:=C
	END;
	BEGIN
	  I:=10;
	  WHILE I>0 DO
	    BEGIN
	      BEGIN
	      END;
	      WRITE(I);
	      I:=I-1
	    END
	END.

## T2.PL0 文件：

	CONST I=5, B=20; C=1, D=2;
	VAR I,J; X,Y;
	PROCEDURE J;
	BEGIN
	  J:=I
	END;
	PROCEDURE K;
	BEGIN
	  J:=I
	END;
	BEGIN
	  WRITE(I);
	  I:=1;
	  WHILE I<10 DO
	    BEGIN
	      BEGIN
	      END;
	      WRITE(I);
	      I:=I+1
	    END
	END.