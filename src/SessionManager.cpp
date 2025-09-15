/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 10:00:00 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/01/27 10:00:00 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/SessionManager.hpp"
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

SessionManager::SessionManager(const std::string& session_dir) 
	: session_dir(session_dir)
{
	struct stat st;
	if (stat(session_dir.c_str(), &st) != 0)
		mkdir(session_dir.c_str(), 0755);
}

SessionManager::~SessionManager() {}

static std::string trim(const std::string& s)
{
	size_t a = s.find_first_not_of(" \n\r\t");
	size_t b = s.find_last_not_of(" \n\r\t");
	if (a == std::string::npos) return "";
	return s.substr(a, b - a + 1);
}

bool	SessionManager::loadJson(const std::string& session_id, SessionData& out)
{
	std::string file_path = session_dir + "/" + session_id + ".json";
	std::ifstream file(file_path.c_str());
	if (!file.is_open())
		return false;
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::string u;
	std::string ls;
	int vc = 0;
	for (size_t i = 0; i < content.size(); ++i)
	{
		if (content.compare(i, 12, "\"username\"") == 0)
		{
			size_t q = content.find('"', content.find(':', i) + 1);
			size_t q2 = content.find('"', q + 1);
			u = content.substr(q + 1, q2 - q - 1);
		}
		else if (content.compare(i, 14, "\"login_status\"") == 0)
		{
			size_t p = content.find(':', i);
			size_t e = content.find_first_of(",}\n\r\t ", p + 1);
			ls = trim(content.substr(p + 1, e - p - 1));
		}
		else if (content.compare(i, 14, "\"visit_count\"") == 0)
		{
			size_t p = content.find(':', i);
			size_t e = content.find_first_of(",}\n\r\t ", p + 1);
			vc = std::atoi(trim(content.substr(p + 1, e - p - 1)).c_str());
		}
	}
	out.username = u;
	out.login_status = (ls == "true");
	out.visit_count = vc;
	return true;
}

void	SessionManager::saveJson(const std::string& session_id, const SessionData& data)
{
	std::string file_path = session_dir + "/" + session_id + ".json";
	std::ofstream file(file_path.c_str());
	if (!file.is_open())
		return;
	file << "{\n";
	file << "\t\"username\": \"" << data.username << "\",\n";
	file << "\t\"login_status\": " << (data.login_status ? "true" : "false") << ",\n";
	file << "\t\"visit_count\": " << data.visit_count << "\n";
	file << "}\n";
	file.close();
}

std::string	SessionManager::createSession()
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	std::stringstream ss;
	ss << "ws_" << std::time(NULL) << "_" << (std::rand() % 100000);
	std::string id = ss.str();
	SessionData init;
	init.username = "";
	init.login_status = false;
	init.visit_count = 0;
	saveJson(id, init);
	return id;
}

SessionData*	SessionManager::getSession(const std::string& session_id)
{
	if (session_id.empty())
		return NULL;
	if (!loadJson(session_id, cached))
		return NULL;
	return &cached;
}

void	SessionManager::updateSessionAccess(const std::string& session_id)
{
	if (session_id.empty())
		return;
	std::string file_path = session_dir + "/" + session_id + ".json";
	struct stat st;
	if (stat(file_path.c_str(), &st) != 0)
		return;
	struct utimbuf times;
	times.actime = st.st_atime;
	times.modtime = st.st_mtime + 1;
	utime(file_path.c_str(), &times);
}

void	SessionManager::cleanupExpired(int max_idle_seconds)
{
	DIR* dir = opendir(session_dir.c_str());
	if (!dir)
		return;
	time_t now = std::time(NULL);
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		const char* name = entry->d_name;
		size_t len = strlen(name);
		if (len > 5 && std::string(name + len - 5) == ".json")
		{
			std::string file_path = session_dir + "/" + name;
			struct stat st;
			if (stat(file_path.c_str(), &st) == 0)
			{
				if ((now - st.st_mtime) > max_idle_seconds)
					unlink(file_path.c_str());
			}
		}
	}
	closedir(dir);
} 
