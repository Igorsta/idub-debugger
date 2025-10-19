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
			- operacje na rejestrach: __operacje arytmetyczne na liczbach całkowitych (zmienno przecinowkoe), operacje na wirtualnych wskaźnikach, control-flow programu__
			- typy rejsestrów: __liczba całkowita ze znakiem i bez (liczby zmiennoprzecinkowe), wirtualne wskaźniki__
		- wywyoływanie funkcji:
			- sposób przekazywania *n* argumentów funkcji: __przez pierwsze  *n* rejestrów__
			- czy funkcję zawsze trzeba wywołać: __tak. nie robimy rekurencji ogonowej ani funkcji inline__
			- podawanie parametrów: 
	- parsowanie pliku
		- usunięcie komentarzy
		- określenie wszystkich etykiet do skoków warunkowych
		- określenie potrzebnej ilości rejestrów dla każdej funkcji
		- (statyczne sprawdzanie typów)
		- walidacja funkcji
		- produkowanie wykonywalnego bytecodu 
	- wykonywanie instrukcji
		- dynamiczne sprawdzanie typów
		- zapewnienie bezpiecz	
	