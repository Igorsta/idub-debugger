1. Dotychczasowe maszyny wirtualne:
	- GDB
	- LLDB
	- ocamldebug
	- PDB

2. Postanowanienia odnośnie projektu:
	- szczegóły techniczne:
		- maszyna stosowa czy rejestrowa: __rejestrowa__
		- rejestry
			- ilość wirtualnych rejestrów: __nieograniczona__
			- odwoływanie się do rejestrów: __${nazwa_funkcji}_1 ... dla kolejnych argumentów funkcji, ${nazwa użytkownika} dla rejstrów programisty__
			- operacje na rejestrach: __operacje arytmetyczne na liczbach całkowitych (zmienno przecinowkoe), operacje na wirtualnych wskaźnikach (zarówno dla stosu jak i stert), control-flow programu__
			- typy rejsestrów: __liczba całkowita ze znakiem i bez (liczby zmiennoprzecinkowe), wirtualne wskaźniki __
		- wywyoływanie funkcji:
			- sposób przekazywania *n* argumentów funkcji: __przez pierwsze  *n* rejestrów__
			- czy funkcję zawsze trzeba wywołać: __tak. nie robimy rekurencji ogonowej ani funkcji inline__
			- podawanie parametrów: 
	- etapy działania interpretera:
		- odczytanie pliku
		- 
	