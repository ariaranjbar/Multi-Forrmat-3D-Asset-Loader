#include <sstream>
#include <fstream>
#include <iostream>
#include "wavefront.h"
#include <string.h>

wavefront::Object wavefront::parse_obj_file(std::string filename)
{
	wavefront::Object objs;
	
	std::fstream file;
	file.open(filename);
	if (!file.is_open())
	{
		std::cout << "error: wavefront parser failed to open \"" << filename << "\" and returned empty object." << std::endl;
		return objs;
	}

	std::stringstream ss;
	ss << file.rdbuf();
	file.close();
	std::string line;
	bool missing_normals = false;
	bool missing_UVs = false;
	while (getline(ss, line))
	{
		if (line[0] == '#' || line.empty())
		{
			continue;
		}
		//std::cout << line << std::endl;
		std::vector<std::string> parsed_line = line_to_words(line, " ");
		if (parsed_line[0] == "g")
		{
			objs.m_SubObjects.push_back(parsed_line[1]);
		}

		if (parsed_line[0] == "v")
		{
			objs.m_Vertices.push_back(glm::vec3(std::stof(parsed_line[1]), std::stof(parsed_line[2]), std::stof(parsed_line[3])));
		}

		if (parsed_line[0] == "vt")
		{
			objs.m_UVs.push_back(glm::vec2(std::stof(parsed_line[1]), std::stof(parsed_line[2])));
		}

		if (parsed_line[0] == "vn")
		{
			objs.m_Normals.push_back(glm::vec3(std::stof(parsed_line[1]), std::stof(parsed_line[2]), std::stof(parsed_line[3])));
		}

		if (parsed_line[0] == "f")
		{
			
			std::vector<glm::ivec3> poly;
			for (int i = 0; i < parsed_line.size() - 1; i++)
			{
				std::stringstream vertexStream;
				std::vector<std::string> indvec = parse_by_delim(parsed_line[i + 1], "/");
				//std::cout << "indvec size: " << indvec.size() << std::endl;
				//std::cout << "indvec 0: " << indvec[0] << std::endl;
				//std::cout << "indvec 1: " << indvec[1] << std::endl;
				//std::cout << "indvec 2: " << indvec[2] << std::endl;
				int V_ind = std::stoi(indvec[0]);

				int T_ind;
				try
				{
					T_ind = std::stoi(indvec[1]);
				}
				catch (const std::invalid_argument& e)
				{
					T_ind = -1;
					missing_UVs = true;
				}

				int N_ind;
				try
				{
					N_ind = std::stoi(indvec[2]);
				}
				catch (const std::invalid_argument& e)
				{
					N_ind = -1;
					missing_normals = true;
				}

				poly.push_back(glm::ivec3(V_ind - 1, T_ind - 1, N_ind - 1));
			}
			if (objs.m_SubObjects.empty())
			{
				objs.m_SubObjects.push_back(std::string("defult"));
			}
			objs.m_SubObjects.back().POLYS.push_back(poly);
		}
	}
	if (missing_UVs)
	{
		std::cout << "!!missing texture coodinations detected!!" << std::endl;
	}
	if (missing_normals)
	{
		std::cout << "!!missing normals detected!!" << std::endl;
		std::cout << "attempting normal reconstruction..." << std::endl;
		objs.calculate_normals();
	}

	return objs;
}

std::vector<std::string> wavefront::line_to_words(std::string str, std::string delim_list)
{
	std::vector<std::string> words;
	char* cstr = new char[str.size()+1];
	for (int i = 0; i < str.size(); i++)
	{
		cstr[i] = str[i];
	}
	cstr[str.size()] = '\0';
	char* pch;
	char* next_token;
	pch = strtok_s(cstr, delim_list.c_str(), &next_token);
	while (pch != NULL)
	{
		words.push_back(pch);
		pch = strtok_s(NULL, delim_list.c_str(), &next_token);
	}
	delete[] cstr;
	return words;
}

std::vector<std::string> wavefront::parse_by_delim(std::string str, std::string delim_list)
{
	std::vector<std::string> parsed;
	if (str == "")
	{
		parsed.push_back("");
		return parsed;

	}
	int start_index = 0;
	int end_index = 0;
	bool ends_in_value = false;
	while (start_index != str.size())
	{
		std::string t;
		bool end_token = false;
		if (end_index == str.size())
		{
			ends_in_value = true;
			end_token = true;
		}
		else
		{
			char str_ch = str.at(end_index);
			for (char ch : delim_list)
			{
				if (str_ch == ch)
				{
					end_token = true;
				}
			}
		}
		end_index++;
		if (end_token)
		{
			if (end_index == start_index + 1)
			{
				parsed.push_back("");
			}
			else
			{
				t = str.substr(start_index, end_index - start_index - 1);
				parsed.push_back(t);
			}
			start_index = end_index;
		}
		if (ends_in_value)
		{
			break;
		}
	}
	if (!ends_in_value)
	{
		parsed.push_back("");
	}
	return parsed;
}

void wavefront::Object::calculate_normals()
{
	std::vector<glm::vec3> vertex_normals(m_Vertices.size(), glm::vec3(0,0,0));
	int k = 0;

	for (auto& s : m_SubObjects)
	{
		for (auto& p : s.POLYS)
		{
			for (int i = 0; i < p.size() - 2; i++)
			{
				glm::vec3 face_normal = 
					glm::cross(m_Vertices[p[i + 1].x] - m_Vertices[p[0].x],
					m_Vertices[p[i + 2].x] - m_Vertices[p[0].x]);
				
				//std::cout << k << ": " << "vert inds: "
				//	<< p[0].x << ", " << p[i + 1].x << ", " << p[i + 2].x << std::endl
				//	<< "vert1: [" << m_Vertices[p[0].x].x << ", " << m_Vertices[p[0].x].y << ", " << m_Vertices[p[0].x].z << "]" << std::endl
				//	<< "vert2: [" << m_Vertices[p[i + 1].x].x << ", " << m_Vertices[p[i + 1].x].y << ", " << m_Vertices[p[i + 1].x].z << "]" << std::endl
				//	<< "vert3: [" << m_Vertices[p[i + 2].x].x << ", " << m_Vertices[p[i + 2].x].y << ", " << m_Vertices[p[i + 2].x].z << "]" << std::endl
				//	<< "face normal: [" << face_normal.x << ", " << face_normal.y << ", " << face_normal.z << "]" << std::endl
				//	<< "face normal length: " << glm::length(face_normal) << " = " << glm::sqrt(face_normal.x * face_normal.x + face_normal.y * face_normal.y + face_normal.z * face_normal.z)
				//	<< std::endl;

				glm::vec3 current_n = vertex_normals[p[0].x];
				if ((glm::length(current_n)) < (glm::length(face_normal)))
				{
					vertex_normals[p[0].x] = face_normal;
					//std::cout << k << ": " << (current_n.length()) << " < " << (face_normal.length()) << std::endl;
				}
				current_n = vertex_normals[p[i + 1].x];
				if ((glm::length(current_n)) < (glm::length(face_normal)))
				{
					vertex_normals[p[i + 1].x] = face_normal;
					//std::cout << k << ": " << (current_n.length()) << " < " << (face_normal.length()) << std::endl;
				}
				current_n = vertex_normals[p[i + 2].x];
				if ((glm::length(current_n)) < (glm::length(face_normal)))
				{
					vertex_normals[p[i + 2].x] = face_normal;
					//std::cout << k << ": " << (current_n.length()) << " < " << (face_normal.length()) << std::endl;
				}
				//k++;
			}
		}
	}
	for (auto& s : m_SubObjects)
	{
		for (auto& p : s.POLYS)
		{
			for (auto& i : p)
			{
				i.z = i.x;
			}
		}
	}
	std::vector<glm::vec3> normals;
	for (int i = 0; i < vertex_normals.size(); i++)
	{
		normals.push_back(glm::normalize(vertex_normals[i]));
	}
	m_Normals = normals;
}

void wavefront::Object::print() const
{
	using namespace std;
	cout << fixed;
	for (auto i : m_SubObjects)
	{
		cout << "sub: " << i.NAME << endl;
		int count = 0;
		for (auto j : i.POLYS)
		{
			cout << "	face " << count << ":" << endl;
			count++;
			for (auto k : j)
			{
				cout << "		V: " << m_Vertices[k.x].x << " " << m_Vertices[k.x].y << " " << m_Vertices[k.x].z << endl;
				cout << "		T: " << m_UVs[k.y].x << " " << m_UVs[k.y].y << endl;
				cout << "		N: " << m_Normals[k.z].x << " " << m_Normals[k.z].y << " " << m_Normals[k.z].z << endl;
				cout << endl;
			}
		}
	}
}
