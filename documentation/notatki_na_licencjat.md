1. Rzeczy, które musimy zrobić:
	- zrobić strukturę trzymającą breakpointy, zmiany w kodzie stworzone w trakcie debugowania oraz dane profilera i nawijania historii wykonania
	- ustalić jak będziemy wykonywać opcody (czy robimy oddzielny zestaw instrukcji, czy bazujemy na "zwykłym" bytecodzie)
	- zrobić mapowanie między instrukcjami bytecodu, a zawartością pliku (coś à la CompilerExplorer) <- TO MOŻE WYKRACZAĆ POZA VM-kę
	- zrobić zestaw komend do CLI, żeby było po Bożemu
	- zrobić jakiś UI + dobrze żeby debugger też miał jakieś konkretne API, które mogą inni wykorzystywać (naprzykład LSP)

2. Istniejące rzeczy w projekcie, które mogą się przydać:
	- cały REPL: dev/src/vm/src/vm/vm_repl.cpp
	- API vm-ki: dev/src/vm/src/vm/api/vm.cpp
	CLI:
		- tokenizacja poleceń:
			- tokenizeFile: dev/src/common/tokenizer/lexer/src/lexer/lexer.cpp:14
			- setKeywordMode: dev/src/common/tokenizer/lang_definitions/src/lang_definitions/key_spec_op.cpp:15
	step-by-step:
		- TokenSource::lines: dev/src/common/tokenizer/token_source/src/token_source/source.cpp:35
		- getCurrentPosition: dev/src/vm/src/vm/api/vm.cpp:144
	breakpoints:
		- vmthread.cpp: dev/src/vm/src/vm/core/thread/vmthread.hpp
		- CodePostion: dev/src/vm/src/vm/api/data/response.hpp:49
	backtrace and stack-analysis:
		- ThreadStack: dev/src/vm/src/vm/core/process/memory/thread_stack.hpp
	function swap/bytecode loading:
		- Compiler: dev/src/vm/src/vm/loader/compiler/compiler.hpp:23
	mapowanie między instrukcjami a pozycjami w pliku:
		- SourcePosition: dev/src/common/diagnostic/src/diagnostic/source_position.hpp:53

2.5. Notatki ze staży (ni w ząb nie wiem co miałem na myśli je pisząc):
	thread_stack.hpp:ThreadStack 
	vmthread.cpp:executeOneStep()
	vmthread.cpp:handleBreakpoint()
	vmthread.cpp:waitForStoppedResponse()
	vmthread.cpp:waitForBreakpointResponse()
	vmhread.cpp:OpFuns:handle_execution_break(OPFUN_ARGS)
	opcode_functions.hpp:performFunctionCall()
	response.hpp:CodePosition	
	source_position.hpp:SourcePosition
	highlight_positions.cpp:printHighlightedPositions()
	elements.hpp:AsmElement
	debug_print.hpp:nullAwareDprint()
	clah_class.cpp:


3. Time-travel debugger:
	- debugger cofający sie w czasie będzie dużo cięższy niż zwykłe wykonanie kodu
	- trzeba przeczytać papiery na ten temat
	- ... i bawić się eksperymentalną implementacją. 
	