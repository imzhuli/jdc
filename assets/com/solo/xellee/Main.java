package com.solo.xellee;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.util.function.Consumer;
import java.util.function.Function;

import com.solo.xellee.sub.Annotation;
import com.solo.xellee.sub.Export;
import com.solo.xellee.sub.Interface;

@Annotation
public abstract class Main implements Interface {
	
	// constant_value
	public static final boolean B = true;
	public static final int  I = 0x12345678;
	public static final long L = 0x1234567890123456L;
	public static final float F = 0.12345f;
	
	// void <clinit>()
	public static volatile double D = 0.1234567890;
	public static final int [] AA = { 1, 2, 3 };
	
	// void <init> ()
	public String S = "ThisIsString";
	
	// Method Handle: typed, directly executable reference to an underlying method, constructor, field, or similar low-level operation, 
	// with optional transformations of arguments or return values.
	public static MethodHandle MH = null;
	
	public enum InnerEnum {
		INNER_ONE,
		INNER_TWO,
		INNER_THREE,
	}
	
	public static void main(String [] Args)
	{
		try {
			Function<Integer, Void> A = null;
		    Consumer<Integer> method = (n) -> { System.out.println(n); };
		    method.accept(10);
		    
		    MethodType mt = MethodType.fromMethodDescriptorString("([Ljava/lang/String;)V", null);
		    System.out.println(mt);
			MH = MethodHandles.lookup().findStatic(Main.class, "main", mt);
		} catch (NoSuchMethodException | IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		System.out.println(Export.S);
	}
	
	public static void varArgs(String ... Args)
	{
	}
	
	public static synchronized void params(String S, int I, Integer javaInteger, boolean [][] Bools, char C)
	{
		
	}
	
	public abstract  void abs(String S, int I, Integer javaInteger, boolean [][] Bools, char C);


}
