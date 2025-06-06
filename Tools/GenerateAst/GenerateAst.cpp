// GenerateAst.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <regex>
#include <sstream>
#include <variant>
#include <unordered_map>
#include <unordered_set>

struct field
{
	std::string type;
	std::string name;
};

std::unordered_map<std::string, std::string>	DefinedTypes;

void defineVisitor(std::ofstream& writer, const std::string& baseName, const std::vector<std::string>& fieldList)
{
	writer << "class " << baseName << "Visitor\n{\npublic:\n";
	std::string lowerBaseName;
	lowerBaseName.resize(baseName.size());
	std::transform(baseName.begin(), baseName.end(), lowerBaseName.begin(), [](unsigned char c) { return std::tolower(c); });
	for (const std::string& type : fieldList)
	{
		std::string typeName{ type.begin(), std::find_if(type.begin(), type.end(), [](unsigned char ch) { return std::isspace(ch); }) };
		
		writer << "\tvirtual Object visit" << typeName << baseName << "(class " << typeName << "& " << lowerBaseName << ") = 0;" << std::endl;
	};

	writer << "\tvirtual ~" << baseName << "Visitor() = default; " << std::endl;
	writer << "};" << std::endl << std::endl;

	DefinedTypes.emplace(std::make_pair(baseName + "Visitor", baseName + ".hpp"));
}

void defineType(std::ofstream& writer, const std::string& baseName, const std::string& className, const std::string& fieldList)
{
	writer << "class " << className << " : public " << baseName << "\n{\npublic:\n\tvirtual ~" << className << "() override {}; " << std::endl;
	
	std::string lowerBaseName;
	lowerBaseName.resize(baseName.size());
	std::transform(baseName.begin(), baseName.end(), lowerBaseName.begin(), [](unsigned char c) { return std::tolower(c); });
	
	// Storing fields
	std::vector<field> fields;
	std::stringstream ss{ fieldList };
	std::string s;
	while (std::getline(ss, s, ','))
	{
		while (std::isspace(s[0]))
			s = s.substr(1, s.size() - 1);
		size_t it = s.find(' ');
		field f;
		f.name = s.substr(it + 1, s.size() - it - 1);
		f.type = s.substr(0, it);
		fields.push_back(f);
	}

	// Constructor:
	writer << "\n\t" << className << "("; 
	bool isFirst = true;
	for (const field& f : fields)
	{
		if (!isFirst)
			writer << ", ";
		isFirst = false;
		writer << "const std::shared_ptr<" << f.type << ">& "<< f.name;
	}
	writer << ") : ";
	isFirst = true;
	for (const field& f : fields)
	{
		if (!isFirst)
			writer << ", ";
		isFirst = false;
		writer << f.name << "{ " << f.name << " }";
	}
	writer << "\n\t{}\n";

	// Storing fields in the class
	for (const field& f : fields)
	{
		writer << "\tstd::shared_ptr<" << f.type << ">\t" << f.name << ";" << std::endl;
	}

	// Visitor pattern
	writer << std::endl << "virtual Object accept(" << baseName << "Visitor& visitor) \n\t{\n";
	writer << "\t\treturn visitor.visit" << className << baseName << "(*this);\n\t}" << std::endl;

	writer << "}; " << std::endl << std::endl;

	DefinedTypes.emplace(std::make_pair(className, baseName + ".hpp"));
}

void defineIncludes(std::ofstream& writer, const std::string& baseName, const std::vector<std::string>& types)
{
	writer << "#pragma once" << std::endl;

	// Include for many base types likely used in this context
	writer << "#include \"Token.h\"" << std::endl;

	std::unordered_set<std::string> requiredFiles;

	// need to include all types generated previously if they are needed, cannot broadly include them as they are generated!
	for (const std::string& it : types)
	{
		auto fieldsDelimitator{ it.find(':') };
		std::stringstream fields{ it.substr(fieldsDelimitator + 1) };

		std::string s;
		while (!fields.eof())
		{
			std::getline(fields, s, ' ');
			if (s.size() == 0 || s == " ")
				continue;
			auto found = DefinedTypes.find(s);
			if (found != DefinedTypes.end())
			{
				requiredFiles.insert(found->second);
			}
			// Skip arg name
			while (!fields.eof() && fields.get() != ',');
		}
	}

	for (const std::string& it : requiredFiles)
	{
		writer << "#include \"" << it << "\"" << std::endl;
	}


	writer << "#include <memory>" << std::endl;
	writer << "" << std::endl;
}

void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types)
{
	std::string path = outputDir + "/" + baseName + ".hpp";
	
	std::ofstream writer{ path };
	
	defineIncludes(writer, baseName, types);

	defineVisitor(writer, baseName, types);

	writer << "class " << baseName << "\n{\npublic:\nvirtual ~" << baseName << "() = default; \n";
	writer << "\tvirtual Object accept(" << baseName << "Visitor& visitor) = 0; " << std::endl;
	
	writer << "}; " << std::endl << std::endl;


	for (std::string type : types)
	{
		std::stringstream stream{ type };
		std::string className, fields;
		std::getline(stream, className, ':');
		className.erase(std::find_if(className.rbegin(), className.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), className.end());
		std::getline(stream, fields, ':');

		defineType(writer, baseName, className, fields);
	}

	DefinedTypes.emplace(std::make_pair(baseName, baseName + ".hpp"));
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: GenerateAst <output directory>" << std::endl;
		exit(64);
	}
	std::cout << "Generating AST expr.hpp" << std::endl;
	std::string outputDir = argv[1];
	defineAst(outputDir, "Expr",
		{
			"Binary   : Expr left, Token op, Expr right",
			"Grouping : Expr expression",
			"Literal  : Object value",
			"Unary    : Token op, Expr right",
			"Variable : Token name"
		});
	defineAst(outputDir, "Stmt",
		{
			"Expression : Expr expression",
			"Print      : Expr expression",
			"Var        : Token name, Expr initializer"
		});
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
