#include "include/crow_all.h"
#include "include/ticket_manager.hpp"
#include <cpr/body.h>
#include <cpr/cpr.h>
#include <memory>
#include <optional>

const std::string ip = "192.168.224.128";
const std::string port = "3000";

const std::string proxy_ip = "192.168.12.120";
const std::string proxy_port = "3000";

int main() {
  crow::SimpleApp app;

  std::string base_url = "http://" + proxy_ip + ":" + port;
  auto ticketManager = std::make_unique<TicketDataManager>(base_url);

  // Rota de login
  CROW_ROUTE(app, "/login")
      .methods("OPTIONS"_method,
               "POST"_method)([&ticketManager](const crow::request &req) {
        crow::response res;

        auto x = crow::json::load(req.body);
        if (!x) {
          res.code = 400;
          res.body = "Invalid JSON";
          return res;
        }

        std::string login = x["Login"].s();
        std::string password = x["Senha"].s();
        std::cout << login << std::endl;
        std::cout << x["Login"].s() << std::endl;

        auto user = ticketManager->getUserByLogin(login);
        if (!user.has_value()) {
          res.code = 204;
          res.body = "User not found";
          return res;
        }

        if (user->senha ==
            password) { // Na prática, use comparação de hash segura
          crow::json::wvalue response_body({{"message", "Login successful"},
                                            {"user",
                                             {{"id", user->id},
                                              {"login", user->login},
                                              {"adm", user->adm}}}});
          res.code = 200;
          res.body = response_body.dump();
        } else {
          res.code = 203;
          res.body = "Incorrect password";
        }

        return res;
      });

  // Rota para registro de usuário
  CROW_ROUTE(app, "/register")
      .methods("POST"_method)([&ticketManager](const crow::request &req) {
        auto x = crow::json::load(req.body);
        if (!x)
          return crow::response(400, "Invalid JSON");

        TicketDataManager::PessoaDAO newUser{x["Login"].s(), x["Senha"].s()};

        try {
          /*
          auto createdUser = ticketManager->createUser(newUser);
          crow::json::wvalue response_body(
              {{"message", "User created successfully"},
               {"user",
                {{"Login", createdUser.login}, {"ADM", createdUser.adm}}}});
                */
          return crow::response(201);
        } catch (const std::exception &e) {
          std::cout << e.what() << std::endl;
          return crow::response(500, "Error creating user: " +
                                         std::string(e.what()));
        }
      });

  // Rota para obter todos os tickets
  CROW_ROUTE(app, "/tickets")
      .methods("GET"_method)([&ticketManager](const crow::request &) {
        try {
          auto tickets = ticketManager->getAllTickets();
          std::cout << "Pegou tickets" << std::endl;
          crow::json::wvalue response_body;
          response_body["tickets"] = crow::json::wvalue::list();
          for (size_t i = 0; i < tickets.size(); ++i) {
            const auto &ticket = tickets[i];
            response_body["tickets"][i] =
                crow::json::wvalue({{"id", ticket.id},
                                    {"titulo", ticket.titulo},
                                    {"descricao", ticket.descricao},
                                    {"prioridade", ticket.prioridade},
                                    {"id_pessoa", ticket.id_pessoa.value_or(0)},
                                    {"status", ticket.status}});
          }
          return crow::response(200, response_body);
        } catch (const std::exception &e) {
          return crow::response(500, "Error fetching tickets: " +
                                         std::string(e.what()));
        }
      });

  // Rota para criar um novo ticket
  CROW_ROUTE(app, "/tickets")
      .methods("POST"_method)([&ticketManager](const crow::request &req) {
        auto x = crow::json::load(req.body);
        if (!x)
          return crow::response(400, "Invalid JSON");

        TicketDataManager::Ticket newTicket{
            0, // ID será atribuído pelo banco
            x["titulo"].s(),
            x["descricao"].s(),
            static_cast<int>(x["prioridade"].i()),
            x["id_pessoa"].i(), // Já é int64_t, não precisa de conversão
            static_cast<int>(x["status"].i())};

        try {
          auto createdTicket = ticketManager->createTicket(newTicket);
          crow::json::wvalue response_body(
              {{"message", "Ticket created successfully"},
               {"ticket",
                {{"id", createdTicket.id},
                 {"titulo", createdTicket.titulo},
                 {"descricao", createdTicket.descricao},
                 {"prioridade", createdTicket.prioridade},
                 {"id_pessoa", createdTicket.id_pessoa.value_or(0)},
                 {"status", createdTicket.status}}}});
          return crow::response(201, response_body);
        } catch (const std::exception &e) {
          return crow::response(500, "Error creating ticket: " +
                                         std::string(e.what()));
        }
      });

  app.loglevel(crow::LogLevel::DEBUG);
  app.port(8080).multithreaded().run();
}