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
	bool useShared = true;
	std::string enclosingType;
	std::string type;
	std::string name;
};

std::unordered_map<std::string, std::string>	DefinedTypes = {
	{"List", "<list>"},
};
// Types that have a different name because we are using a system library to implement them.
// These won't need a std::shared_ptr too, so other types may be put inside of this later.
std::unordered_map<std::string, std::string>	TypeTranslationMap = {
	{"List", "std::list"},
	{"Shared", "std::shared_ptr"},
};

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

	DefinedTypes.emplace(std::make_pair(baseName + "Visitor", '"' + baseName + ".hpp\""));
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

		// Search for an enclosing type
		if (size_t pos = f.type.find('<'); pos < f.type.length())
		{
			f.enclosingType = f.type.substr(0, pos);
			f.type = f.type.substr(pos + 1);
			// Remove the '>' at the end of the type
			f.type.pop_back();
		}

		// Use translation map to get 
		if (auto it = TypeTranslationMap.find(f.enclosingType); it != TypeTranslationMap.end())
		{
			f.useShared = false;
			f.enclosingType = it->second;
		}
		if (auto it = TypeTranslationMap.find(f.type); it != TypeTranslationMap.end())
		{
			f.useShared = false;
			f.type = it->second;
		}

		if (f.useShared || DefinedTypes.contains(f.type))
			f.type = "std::shared_ptr<class " + f.type + '>';

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
		std::string fullType;
		if (f.enclosingType.size() > 0)
			fullType = f.enclosingType + "<class " + f.type + ">";
		else
			fullType = f.type;

		writer << "const " << fullType << "& "<< f.name;
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
		std::string fullType;
		if (f.enclosingType.size() > 0)
			fullType = f.enclosingType + "<class " + f.type + ">";
		else
			fullType = f.type;
		writer << "\t" << fullType << "\t" << f.name << ";" << std::endl;
	}

	// Visitor pattern
	writer << std::endl << "virtual Object accept(" << baseName << "Visitor& visitor) \n\t{\n";
	writer << "\t\treturn visitor.visit" << className << baseName << "(*this);\n\t}" << std::endl;

	writer << "}; " << std::endl << std::endl;

	DefinedTypes.emplace(std::make_pair(className, '"' + baseName + ".hpp\""));
}

void defineIncludes(std::ofstream& writer, const std::string& baseName, const std::vector<std::string>& types)
{
	writer << "#pragma once" << std::endl;

	// Include for many base types likely used in this context
	writer << "#include \"Token.h\"" << std::endl;
	writer << "#include \"Object.h\"" << std::endl;

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
			while (std::isspace(s[0]))
				s = s.substr(1, s.size() - 1);
			size_t it = s.find(' ');
			field f;
			f.type = s.substr(0, it);

			// Search for an enclosing type
			if (size_t pos = f.type.find('<'); pos < f.type.length())
			{
				f.enclosingType = f.type.substr(0, pos);
				f.type = f.type.substr(pos + 1);
				// Remove the '>' at the end of the type
				f.type.pop_back();
			}

			if (auto found = DefinedTypes.find(f.type); found != DefinedTypes.end())
				requiredFiles.insert(found->second);
			if (auto found = DefinedTypes.find(f.enclosingType); found != DefinedTypes.end())
				requiredFiles.insert(found->second);
			// Skip arg name
			while (!fields.eof() && fields.get() != ',');
		}
	}

	for (const std::string& it : requiredFiles)
	{
		writer << "#include " << it << std::endl;
	}


	writer << "#include <memory>" << std::endl;
	writer << "" << std::endl;
}

void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types)
{
	std::string path = outputDir + "/" + baseName + ".hpp";
	
	std::ofstream writer{ path };
	
	defineIncludes(writer, baseName, types);
	
	// Only add a defined type after generating the includes to not have a potential self include
	DefinedTypes.emplace(std::make_pair(baseName, '"' + baseName + ".hpp\""));

	defineVisitor(writer, baseName, types);

	writer << "class " << baseName << "\n{\npublic:\nvirtual ~" << baseName << "() = default; \n";
	writer << "\tvirtual Object accept(" << baseName << "Visitor& visitor) = 0; " << std::endl;
	
	writer << "}; " << std::endl << std::endl;

	for (const std::string& type : types)
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

}

int main(int argc, char* argv[])
{
	std::string outputDir;
	if (argc != 2)
	{
		std::cout << "Usage: GenerateAst <output directory>" << std::endl;
		//exit(64);
		outputDir = "C:\\Users\\hagry\\Documents\\ProjetsPerso\\CraftingInterpreters\\TreeWalk\\include";
	}
	else
		outputDir = argv[1];
	std::cout << "Generating AST expr.hpp" << std::endl;
	defineAst(outputDir, "Stmt",
		{
			"Block      : List<Stmt> statements",
			"Expression : Expr expression",
			"If         : Expr condition, Stmt thenBranch, Stmt elseBranch",
			"Print      : Expr expression",
			"Var        : Token name, Expr initializer"
		});
	defineAst(outputDir, "Expr",
		{
			"Assign   : Token name, Expr value",
			"Binary   : Expr left, Token op, Expr right",
			"Grouping : Expr expression",
			"Literal  : Object value",
			"Logical  : Expr left, Token op, Expr right",
			"Unary    : Token op, Expr right",
			"Variable : Token name"
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
