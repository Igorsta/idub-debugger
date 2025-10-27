1. Rzeczy, które musimy zrobić:
	- __zrobić strukturę dla metadanych programowania__
		- __breakpointy__
		- zmiany w kodzie stworzone w trakcie debugowania 
		- dane profilera 
			- __ilość wykonania danej instrukcji__
			- ilość wywołania danej funkcji
			- ilość wykonanych skoków
			- cokolwiek co innego zechce ZPP od JIT-a
			- __mierzenie czasu wykonania__
		- nawijana historia wykonania
	- ustalić jak będziemy wykonywać opcody 
		- czy potrzubujemy oddzielny zestaw instrukcji do debugowania
	- __zrobić mapowanie między instrukcjami bytecodu, a zawartością pliku__
		- coś à la CompilerExplorer
		- 
		- DLA PLIKÓW  TO ZNACZĄCO WYKRACZA POZA VM-kę
	- zrobić zestaw komend do CLI, żeby było po Bożemu
		- __standardowe nazewnictwo__
			- next(n)
			- step(s)
			- nexti(ni)
			- start
			- run(r)
			- finish(fin)
			- backtrace(bt)
			- break(b)
			- info
	- zrobić jakiś UI + dobrze żeby debugger też miał jakieś konkretne API, które mogą inni wykorzystywać (naprzykład LSP)
		- +Tab podpowiadał symbole
		- +Enter wykonuje ostatnie polecenie
		- "define" pozwala na wprowadzanie własnych instrukcji i łączyć podstawowe instrukcje jak np. "next, print var, "

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
	profiler time measurement:
		- PrintAs: dev/src/common/timer/src/timer/timer.cpp:38

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
	