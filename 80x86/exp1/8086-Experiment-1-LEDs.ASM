   .MODEL SMALL
   .DATA
   .STACK 64
   .CODE
PortOut	EQU	88h	;定义输出端口号

STATE0:	 MOV AL,36H
	 OUT PortOut,AL

STATE1:	 MOV AL,33H
	 OUT PortOut,AL

STATE2:	 MOV AL 33H
	 OUT PortOut,AL
	 MOV AL 37H
	 OUT PortOut,AL
	 MOV AL 35H
	 OUT PortOut,AL
	 
STATE3:  MOV AL 1EH
	 OUT PortOut,AL
	 
STATE0:  MOV AL 1EH
	 OUT PortOut,AL
	 MOV AL 3EH
	 OUT PortOut,AL
	 MOV AL 2EH
	 OUT PortOut,AL


MAIN: CALL STATE0
CYCLE:CALL STATE1
      CALL STATE2
      CALL STATE3
      CALL STATE4
      JMP CYCLE


END	main				;指示汇编程序结束编译
   
