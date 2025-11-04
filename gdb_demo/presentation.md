# GDB

## Powszechne fakty

- Powstał niemal 40 lat temu.
- Jest open-source - stanowi część projektu GNU
- prawdopodobnia najstarszy z powszechnie wykorzystywanych debuggerów
- jest backendem dla wielu innych debuggerów (CLION, WinGDB, [inne](https://sourceware.org/gdb/wiki/GDB%20Front%20Ends))

```
- Nuda, nikogo to nie będzie obchodzić.
- zamieścić linki do źródeł dla ciekawskich
```

## Funkcjonalność GDB

- wspiera wiele języków
	- docelowe: C/C++, Fortran, Assembly
	- poboczne: Rust, Pascal
- ma CLI, GUI, API Machine Interface (dla IDE), API w Pythonie (dla programisty)
- pozwala zapisywać metadane debugowania do plików
- ... i jeszcze dużo, dużo więcej

```
- GDB jest wszechstronne
(?) - planujemy bazować na GDB podczas ZPP-a
- jest wiele fajnyc rzeczy w GDB, które nie będziemy w stanie/mieli czas zaimplmenować
- skupimy się na tym co zrobimy w ramach ZPP
```

## Przykładowy program

```cpp
// gdb_demo.cpp
#include <bits/stdc++.h>

int global_var = 512;

template <typename T>
T xorAdd(T a, T b) {
	a += 8;
	global_var *= 2;
	b -= 7;
	return a + b;
}

int boring_func() {
	return 1042;
}

int func() {
	static int el = 4;	// deklaracja statycznych zmiennych nie wyświteli się w gdb
	int func_local_var = boring_func();
	if (el != 0) {
		el += (el + 1) * (2 * el + 1) * el / 6;
	}
	return el--;
}

int main(int argc, const char *argv[]) {
	int ret = func();
	std::cout << ret << "\n";
	std::cout << xorAdd(ret, 2 * ret) << "\n";
}
```

```
- powinien ilustrować analizowane funkcjonanści GDB
- Przykładowy program jest upośledzony (autor o tym wie)
```

## Kompilacja programu i odpalenie GDB
```bash
g++ gdb_demo.cpp -g -o gdb_demo
gdb ./gdb_demo
```
- wszystko działa!
    ```
    Reading symbols from ./build/bin/gdb_demo...
    (gdb) 
    ```

- flaga *-g* jest ważna

    ```
    Reading symbols from ./gdb_demo...
    (No debugging symbols found in ./gdb_demo)
    ```

```
- kompilujemy z flagą -g, żeby zamieścić w plikuu wykonywalnym symbole debuggowania
- GDB będzie narzekał, że nie znalazł symboli debugowania
- bez symboli debugowania, można polegać jedynie na kodzie maszynowym pliku 
```

## Uruchomienie programu

Są 2 typy ludzi:

- Albo zaczynająsz spokojnie, od początku i po kolei ...
    ```gdb
    (gdb) start ThisIsMainArg
    Starting program: /gdb_demo 
    Temporary breakpoint 1, main (argc=2, argv=...) at /gdb_demo.cpp:27
    27		int ret = func();
    ```
- ... albo dzida do przodu 
    ```gdb
    (gdb) run ThisIsMainArg
    Starting program: /gdb_demo ThisIsMainArg
    ```

```
- w obu przypadkach należy podać argumenty dla main()
- Dla prostoty prezentacji zakładamy, że nie było dzidy,
```

## Wchodzenie do funkcji

- W poprzednium odcinku...

    ```gdb
    ...
    27              int ret = func();
    ```

- ... a teraz możemy zrobić
    ```gdb
    (gdb) s
    func () at /gdb_demo.cpp:19
    19		int func_local_var = boring_func();
    ```

```
- wejście do funkcji robimy poleceniem "step"/"s"
- GDB poinformuje nas o tym do jakiej funkcji weszliśmy
- GDB wyświeli nam pierwszą linijkę, w której będzie coś do zrobienia
- deklaracja zmiennych statycznych NIE JEST czymś do roboty
```

## Przejście do następnej instrukcji

- Coś robimy, może wywołujemy jakąś funkcję, może zapuszczamy DFS-a, ...
    ```gdb
    ...
    19		int func_local_var = boring_func();
    ```

- ... ale nas to nie obchodzi, chcemy jedynie przejść do kolejnej instrukcji
    ```gdb
    (gdb) n
    20		if (el != 0) {
    ```

```
- przejśćie do wykonywanej następnej linii w pliku robimy poleceniem "next"/"n"
- najbardziej podstawowe polecenie w GDB
```

## Wyświetlanie zmiennych w danym konkście

- można wyświetlać wszystkie zmiene danym kontekście
    ```gdb
    (gdb) p global_var 
    $1 = 512
    (gdb) p func_local_var
    $2 = 1042
    ```

- możemy też zobaczyć wszystkie zmienne lokalne dla funkcji
    ```gdb
    (gdb) info locals
    el = 4
    func_local_var = 1042
    ```

- a dolar to przyjaciel, a nie wróg
    ```
    (gdb) p $2
    $3 = 1042
    ```

```
- do wyświetlania nazw zmiennych używamy polecenia "print"/"p"
- zazwyczaj "info locals", gdy zmiennych lokalnych jest mało/są czytelne
- każdy print wprowadza nam zmienną debugowania, którem można wykorzystać jeśli chcemy
```

## Dostęp do stosu wywołań

- wyświetlanie stos wywołań
    ```gdb
    (gdb) bt
    #0  func () at /gdb_demo.cpp:20
    #1  ... in main (argc=2, argv=...) at /gdb_demo.cpp:27
    ```

- dostęp do zmiennych w poprzednich frame'ach
    ```gdb
    (gdb) p argv[1]
    No symbol "argv" in current context.
    (gdb) frame 1
    #1  ... in main (argc=2, argv=...) at /gdb_demo.cpp:27
    27		int ret = func();
    (gdb) p argv[1]
    $4 = ... "ThisIsMainArg"
    ```

```
- podglądamy stos wywołań poleceniem "backtrace"/"bt" a przełączamy się za pomocą polecenia "frame" 
- możemy zobaczyć skąd została wywołana aktualnie wykonywana funkcja
- można sprawdzić jakie są wartości zmiennych doprowadziły do błędu
```

## Dokończeni funkcj

- wracamy do funkcji na czubku stosu wywołań...
    ```gdb
    (gdb) frame 0
    #0  func () at /gdb_demo.cpp:20
    20		if (el != 0) {
    ```

- ... i czekamy aż z niej wyjdziemy
    ```
    (gdb) fin
    Run till exit from #0  func () at /gdb_demo.cpp:19
    ... in main (argc=2, argv=...) at /gdb_demo.cpp:27
    27		int ret = func();
    Value returned is $5 = 34
    ```

```
- wykonanie programu do końca aktualnej funkcji zlecamy polecenim "finish"/"fin"
- GDB pokaże nam miejsce do którego wróciliśmy oraz wynik wykonanej funkcji
- wynik zmiennej trafia do nowej zmiennej debuggowania
```


## Sprawdzanie aktualnie wykonywaną instrukcję

- sprawdzić gdzie aktualnie jestem ...
    ```gdb
    (gdb) frame
    #0  ... in main (argc=2, argv=...) at /gdb_demo.cpp:26
    27		int ret = func();
    ```
- ... i co mnie otacza
    ```gdb
    (gdb) l
    22		}
    23		return el--;
    24	}
    25	
    26	int main(int argc, const char *argv[]) {
    27		int ret = func();
    28		std::cout << ret << "\n";
    29		std::cout << xorAdd(ret, 2 * ret) << "\n";
    30	}
    ```

```
- polecenie "frame", podana bez argumentów, pokaże jedynie gdzie znajduje się czubek stosu wywołań
- do wyświetlenia kodu źródłowego dookoła aktualnie wykonywanej linii służy polecenie "list"/"l"
```

## Sprawdzanie typu

- można sprawdzić typ elementu
	```gdb
	(gdb) whatis ret
	type = int
	```
- nawet jeśli jest szablonem
    ```gdb
    (gdb) whatis add<int>
    type = int (int, int)
    ```

```
- sprawdzanie typu zmiennej robimy poleceniem "whatis"
(?)- chyba nie działa jeśli szablon nie został zainicjalizowany
(?)- chyba nie wspiera polimorfizmu
```

## Podmiana wartości zmiennej
- mamy możliwość podmiany wartości zmiennej tak ...
    ```gdb
    (gdb) set variable ret = 7
    (gdb) p ret
    $6 = 7
    ```
- ... albo tak
    ```
    (gdb) p ret = 42
    $7 = 42
    ```

```
- "set" jest poleceniem debuggera, która pozwala na robić, prawdopodbnie właśnie w ten sposób będziemy robić podmianę zmiennej
- drugie polecenie polega na tym, że wypisując jakieś wyrażenie, musi być najpierw ono ewaluowane, tak więc wypisujemy wartość (ret = 42), co w C++ zwraca nam przypisany wynik
- nie jesteśmy robić trików z ewaluowania składni Ducklinga, dopóki nie mamy REPL-a
(?)- pytanie do szefów czy to się zgadza z ich wizją    
```

## Breakpointy

- na linach:
    ```gdb
    (gdb) b 9
    Breakpoint 2 at ...: file /gdb_demo.cpp, line 9.
    ```
- na funkcjacch:
    ```
    (gdb) b xorAdd<int>(int, int) 
    Breakpoint 3 at ...: file /gdb_demo.cpp, line 7.
    ```


- wyświetlanie dotychczasowych breakpointów:
    ```
    (gdb) info breakpoints 
    Num     Type           Disp Enb Address What
    2       breakpoint     keep y   ...     in xorAdd<int>(int, int) at /gdb_demo.cpp:9
    3       breakpoint     keep y   ...     in xorAdd<int>(int, int) at /gdb_demo.cpp:7

    ```

## Wznawianie wykonywania programu (1)

- Puszczamy hamulce...
    ```
    (gdb) c
    Continuing.
    ```
- ... aż trafimy na pierwszy breakpoint (pierwsza instrukcja funkcji)
    ```gdb
    ...
    
    Breakpoint 3, xorAdd<int> (a=34, b=68) at /gdb_demo.cpp:7
    7		a += 8;
    ```

## Wznawianie wykonywania programu 

- I jeszce raz...
    ```
    (gdb) c
    Continuing.
    ```   
- ... i znowy się zatrzymujemy
    ```gdb
    ...
        
    Breakpoint 2, xorAdd<int> (a=42, b=68) at /gdb_demo.cpp:9
    9		b -= 7;
    ```


## (?) Cofanie się w czasie

```
- wspierane od wersji 7.0
- domyślny limit w cofaniu 20'000 instrukcji
- drogie pamięciowo
- stawia się breakpointy początek i koniec przedziału po którym będzie cofanie, a potem nagrywa 
```
