// BytecodeVirtualMachine.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

#include <iostream>
#include <fstream>
#include <sstream>


#ifdef _MSC_VER 
#include "Windows.h"
#endif

void repl()
{
	char line[1024];
	for (;;) {
		printf("> ");

		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break;
		}

		VM vm;
		vm.interpret(line);
	}
}

void	runFile(std::string path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Could not open file \"" << path << "\"." << std::endl;
		exit(74);
	}
	
	std::string content;

	{// This unfortunately creates a redundant copy, but don't really care about that
		std::ostringstream sstr;
		sstr << file.rdbuf();
		content = sstr.str();
	}

	VM vm;
	InterpretResult result = vm.interpret(content);

	if (result == INTERPRET_COMPILE_ERROR) exit(65);
	if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}



//TODO: Add unit testing (https://github.com/munificent/craftinginterpreters/tree/master/test)	
int main(int argc, const char* argv[])
{
#ifdef _MSC_VER 
	// Set windows console to utf-8 mode to allow printing of utf-8 values
	// This does not fully works because the buffer can still be filled and that will result in a broken character if a utf-8 character was being printed.
	// but is a simple workaround for what is essentially a visual studio and windows bug that "works as designed"

	// Set console code page to UTF-8 so console known how to interpret string data
	SetConsoleOutputCP(CP_UTF8);

	// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
	setvbuf(stdout, nullptr, _IOFBF, 1000);
	setvbuf(stderr, nullptr, _IOFBF, 1000);
#endif
	if (argc == 1)
		//repl();
		runFile("../UnitTesting/test/function/body_must_be_block.lox");
	else if (argc == 2)
		runFile(argv[1]);
	else
	{
		std::cerr << "Usage: BytecodeVirtualMachine [path]" << std::endl;
		exit(64);
	}
	return 0;
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
