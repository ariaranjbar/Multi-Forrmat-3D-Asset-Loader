#pragma once

#include <vector>
#include <string>
#include "glm/glm.hpp"



namespace wavefront
{
	class SubObject
	{
	public:
		std::string NAME;
		std::vector<std::vector<glm::ivec3>> POLYS;
		inline SubObject(std::string name) : NAME(name) {};
	};

	class Object
	{
		friend wavefront::Object parse_obj_file(std::string filename);
	private:
		std::vector<glm::vec3> m_Vertices;
		std::vector<glm::vec2> m_UVs;
		std::vector<glm::vec3> m_Normals;
		std::vector<SubObject> m_SubObjects;
		void calculate_normals();
	public:
		void print() const;
		inline const std::vector<glm::vec3>& vertices() const { return m_Vertices; }
		inline const std::vector<glm::vec2>& UVs() const {return m_UVs;}
		inline const std::vector<glm::vec3>& normals() const {return m_Normals;}
		inline const std::vector<SubObject>& subObjects() const { return m_SubObjects;}
	};

	Object parse_obj_file(std::string filename);
	
	std::vector<std::string> line_to_words(std::string str, std::string delim_list);
	std::vector<std::string> parse_by_delim(std::string str, std::string delim_list);
}

