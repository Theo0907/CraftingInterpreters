// TreeWalk.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "TreeWalk.h"
#include "Expr.hpp"
#include "Token.h"
#include "AstPrinter.h"

// TODO: Add tracy and trace everything for both performance and memory leak
// TODO: Check and prevent circular dependancy of std::shared_ptr (will probably need more advanced code generator to properly take when to use a shared pointer in GenerateAST)

int main(int argc, char* argv[])
{
	/*Binary b{
		std::make_shared<Unary>(
			std::make_shared<Token>(Token(Token::TokenType::MINUS, "-", {}, 1)),
			std::make_shared<Literal>(std::make_shared<Object>(123.0))),
		std::make_shared<Token>(Token(Token::TokenType::STAR, "*", {}, 1)),
		std::make_shared<Grouping>(std::make_shared<Literal>(std::make_shared<Object>(45.67)))
	};

	std::cout << std::get<std::string>(AstPrinter().print(b)) << std::endl;
	*/
	if (argc > 2)
	{
		std::cout << "Usage: twlox [script]" << std::endl;
		exit(64);
	}
	else if (argc == 2)
		TreeWalk::RunFile(argv[1]);
	else
		TreeWalk::RunPrompt();
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
