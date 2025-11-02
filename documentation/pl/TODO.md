0. Disclaimer
	- wykorzystywane z Ducklinga: __pragma DIAGNOSTICS, MUST_INLINE, INLINE, opcode functions__
	- spis rzecz

1. Dotychczasowe maszyny wirtualne:
	- GDB
	- LLDB
	- ocamldebug
	- PDB
	- JDB

2. Szczegóły techniczne maszyny:
	- maszyna stosowa czy rejestrowa: __rejestrowa__
	- rejestry
		- ilość wirtualnych rejestrów: __nieograniczona__
		- dostępność rejestrów: __ograniczona, programista może się odwoływać tylko do rejestrów aktualnie wykonywanej funkcji__
		- możliwość wprowadzenia własnych rejestrów: __wspiarana, programista może stworzyć własne rejestry__
		- odwoływanie się do rejestrów: __${nazwa_funkcji}_0, ${nazwa_funkcji}_1, ${nazwa_funkcji}_2 ... dla kolejnych argumentów funkcji, ${nazwa_programisty} dla rejstrów programisty. Wprowadzona nazwa nie może kolidować z nazwami już istniejącymi__
	- operacje na rejestrach
		- dostępne operacje arytmetyczne: __and, or, neg, xor, +, -, /, *__
		- typy rejestrów: __określone przez instrukcje__
		- operacje porównania: __zainspirowane z Ducklinga__
	- ramka
	- stos pamięci:
		- 
	- funkcje:
		- sposób przekazywania *n* argumentów funkcji: __przez pierwsze  *n* rejestrów (wcześniej opisane)__
		- czy funkcja zawsze jest widoczna na stosie wywołań: __tak. Nie robimy rekurencji ogonowej ani funkcji inline__
		- określanie ilości parametrów: __programista podaje liczbę w nawiasie obok deklaracji funkcji__

3. Postanowanienia odnośnie implementacji projektu:
	- szczegóły techniczne projektu:
		- standard C++23 (potrzebne do this deduction)
		- CMAKE do budowania projektu
		- skrypty wspierjące p
	- parsowanie pliku
		- usunięcie komentarzy
		- określenie wszystkich etykiet w obrębie funkcji
		- określenie potrzebnej ilości rejestrów dla każdej funkcji
		- mapowanie nazw rejestrów na indeksy
		- (statyczne sprawdzanie typów)
		- walidacja funkcji
		- produkowanie wykonywalnego bytecodu 
	- wykonywanie instrukcji
		- dynamiczne sprawdzanie typów
		- sprawdzanie możliwości bezpiecznego wykonania kodu (buffer-overflow itp.)

4. Testowane pomysły na licencjat
	- backtrace
	- podgląd stosu pamięci
	- zliczanie ilości wykonanych instrukcji
	- identyfikowanie instrukcji (unikalny numer identyfikacyjne)
	- mapa numerów identyfikacyjnych na punkty zatrzymania 
	- wątek zdolny do cofania się wstecz wykonywanego kodu (zaawansowane)
		- wiele wątków będących na różnych etapach wykonywania kodu
		- spowalnia wykonywania kodu, natomiast ułatwia znajdywanie powodów błędu 
		- papiery w tej dziedzinie:
			- [https://arxiv.org/abs/1212.5204]
			- [https://arxiv.org/abs/1703.10864]
		
	