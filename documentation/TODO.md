1. Dotychczasowe maszyny wirtualne:
	- GDB
	- LLDB
	- ocamldebug
	- PDB

2./3. Postanowanienia odnośnie projektu:
	- szczegóły techniczne:
		- maszyna stosowa czy rejestrowa: __rejestrowa__
		- rejestry
			- ilość wirtualnych rejestrów: __nieograniczona__
			- dostępność rejestrów: __nieograniczona, z każdej funkcji można odnosić się do rejestrów innej funkcji__
			- możliwość wprowadzenia własnych rejestrów: __wspiarana, programista może stworzyć własne rejestry__
			- odwoływanie się do rejestrów: __${nazwa_funkcji}_0, ${nazwa_funkcji}_1, ${nazwa_funkcji}_2 ... dla kolejnych argumentów funkcji, ${nazwa_programisty} dla rejstrów programisty__
			- operacje na rejestrach: __operacje arytmetyczne na liczbach całkowitych (zmienno przecinkowe), operacje na wirtualnych wskaźnikach, control-flow programu__
			- typy rejsestrów: __liczba całkowita ze znakiem i bez (liczby zmiennoprzecinkowe), wirtualne wskaźniki__
		- wywyoływanie funkcji:
			- sposób przekazywania *n* argumentów funkcji: __przez pierwsze  *n* rejestrów__
			- czy funkcję zawsze trzeba wywołać: __tak. nie robimy rekurencji ogonowej ani funkcji inline__
			- podawanie parametrów: 
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
		
	