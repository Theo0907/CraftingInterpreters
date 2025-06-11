// BytecodeVirtualMachine.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

//TODO: Add unit testing (https://github.com/munificent/craftinginterpreters/tree/master/test)	
int main(int argc, const char* argv[])
{
	Chunk chunk;
	int constant = chunk.addConstant(1.2);
	chunk.write(OP_CONSTANT, 123);
	chunk.write(constant, 123);
	constant = chunk.addConstant(3.4);
	chunk.write(OP_CONSTANT, 123);
	chunk.write(constant, 123);

	chunk.write(OP_ADD, 123);

	constant = chunk.addConstant(5.6);
	chunk.write(OP_CONSTANT, 123);
	chunk.write(constant, 123);

	chunk.write(OP_DIVIDE, 123);
	chunk.write(OP_NEGATE, 123);
	chunk.write(OP_RETURN, 123);
	//disassembleChunk(&chunk, "Chunk");

	{
		VM vm;
		vm.interpret(&chunk);
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
