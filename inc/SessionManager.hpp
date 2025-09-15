/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 10:00:00 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/01/27 10:00:00 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <utime.h>

struct SessionData
{
	std::string	username;
	bool		login_status;
	int			visit_count;
	SessionData() : username(""), login_status(false), visit_count(0) {}
};

class SessionManager
{
	private:
		std::string	 session_dir;
		SessionData	 cached;
		bool		 loadJson(const std::string& session_id, SessionData& out);
		void		 saveJson(const std::string& session_id, const SessionData& data);
	public:
		SessionManager(const std::string& session_dir = "/tmp/webserv_sessions");
		~SessionManager();
		std::string	createSession();
		SessionData* getSession(const std::string& session_id);
		void		updateSessionAccess(const std::string& session_id);
		void		cleanupExpired(int max_idle_seconds);
};

#endif 