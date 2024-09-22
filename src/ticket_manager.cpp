#include "../include/ticket_manager.hpp"
#include <cpr/cpr.h>

#include <cstddef>
#include <stdexcept>

TicketDataManager::Pessoa json_to_pessoa(const crow::json::rvalue &rvalue) {
  auto pessoa = TicketDataManager::Pessoa();
  std::cout << rvalue["ID"].i() << std::endl;
  pessoa.id = rvalue["ID"].i();
  pessoa.login = rvalue["Login"].s();
  std::cout << rvalue["Login"].s() << std::endl;
  pessoa.senha = rvalue["Senha"].s();
  std::cout << rvalue["Senha"].s() << std::endl;

  pessoa.adm = rvalue["ADM"].b();
  std::cout << rvalue["ADM"].b() << std::endl;
  return pessoa;
}

class TicketDataManager::Impl {
public:
  Impl(const std::string &base_url) : base_url(base_url) {}
  
  std::string base_url;

  std::pair<int, crow::json::rvalue>
  makeRequest(const std::string &endpoint, const std::string &method,
              const crow::json::wvalue &data = {}) {
    cpr::Response r;

    if (method == "GET") {
      r = cpr::Get(cpr::Url{base_url + endpoint});
    } else if (method == "POST") {
      r = cpr::Post(cpr::Url{base_url + endpoint}, cpr::Body{data.dump()},
                    cpr::Header{{"Content-Type", "application/json"}});
    } else if (method == "PUT") {
      r = cpr::Put(cpr::Url{base_url + endpoint}, cpr::Body{data.dump()},
                   cpr::Header{{"Content-Type", "application/json"}});
    }

    if (r.status_code >= 200 && r.status_code < 300) {
      return {r.status_code, crow::json::load(r.text)};
    } else {
      throw std::runtime_error("HTTP request failed: " +
                               std::to_string(r.status_code));
    }
  }
};

TicketDataManager::TicketDataManager(const std::string &base_url)
    : pImpl(std::make_unique<Impl>(base_url)) {}

TicketDataManager::~TicketDataManager() = default;
TicketDataManager::TicketDataManager(TicketDataManager &&) noexcept = default;
TicketDataManager &
TicketDataManager::operator=(TicketDataManager &&) noexcept = default;

std::vector<TicketDataManager::Ticket> TicketDataManager::getAllTickets() {
  auto [status, response] = pImpl->makeRequest("/tickets", "GET");
  std::cout << status << std::endl;
  std::vector<Ticket> tickets;
  for (const auto &t : response) {

    if (t["ID_pessoa"].nt() == crow::json::num_type::Null) {
      tickets.push_back({t["ID"].i(), t["Titulo"].s(), t["Descricao"].s(),
                         static_cast<int>(t["Prioridade"].i()), 0,
                         static_cast<int>(t["Status"].i())});
    } else {
      tickets.push_back({t["ID"].i(), t["Titulo"].s(), t["Descricao"].s(),
                         static_cast<int>(t["Prioridade"].i()),
                         t["ID_pessoa"].i(),
                         static_cast<int>(t["Status"].i())});
    }
  }
  return tickets;
}

TicketDataManager::Ticket
TicketDataManager::createTicket(const Ticket &ticket) {
  crow::json::wvalue data({{"Titulo", ticket.titulo},
                           {"Descricao", ticket.descricao},
                           {"Prioridade", ticket.prioridade},
                           {"Status", ticket.status},
                           {"ID_pessoa", ticket.id_pessoa.value_or(0)}});

  auto [status, response] = pImpl->makeRequest("/tickets", "POST", data);
  return {
      response["ID"].i(),        response["Titulo"].s(),
      response["Descricao"].s(), static_cast<int>(response["Prioridade"].i()),
      response["ID_pessoa"].i(), static_cast<int>(response["Status"].i())};
}

TicketDataManager::Ticket
TicketDataManager::updateTicket(int64_t ticket_id, const Ticket &ticket) {
  crow::json::wvalue data({{"Titulo", ticket.titulo},
                           {"Descricao", ticket.descricao},
                           {"Prioridade", ticket.prioridade},
                           {"Status", ticket.status},
                           {"ID_pessoa", ticket.id_pessoa.value_or(0)}});

  auto [status, response] =
      pImpl->makeRequest("/tickets/" + std::to_string(ticket_id), "PUT", data);
  return {
      response["ID"].i(),        response["Titulo"].s(),
      response["Descricao"].s(), static_cast<int>(response["Prioridade"].i()),
      response["ID_pessoa"].i(), static_cast<int>(response["Status"].i())};
}

std::vector<TicketDataManager::Ticket>
TicketDataManager::getCompletedTickets() {
  auto [status, response] = pImpl->makeRequest("/tickets/completos", "GET");
  std::vector<Ticket> tickets;
  for (const auto &t : response) {
    tickets.push_back({t["ID"].i(), t["Titulo"].s(), t["Descricao"].s(),
                       static_cast<int>(t["Prioridade"].i()),
                       t["ID_pessoa"].i(), static_cast<int>(t["Status"].i())});
  }
  return tickets;
}

std::vector<TicketDataManager::Ticket>
TicketDataManager::getIncompleteTickets() {
  auto [status, response] = pImpl->makeRequest("/tickets/nao_completos", "GET");
  std::vector<Ticket> tickets;
  for (const auto &t : response) {
    tickets.push_back({t["ID"].i(), t["Titulo"].s(), t["Descricao"].s(),
                       static_cast<int>(t["Prioridade"].i()),
                       t["ID_pessoa"].i(), static_cast<int>(t["Status"].i())});
  }
  return tickets;
}

std::vector<TicketDataManager::Ticket>
TicketDataManager::getTicketsByUser(int64_t user_id) {
  auto [status, response] =
      pImpl->makeRequest("/tickets/usuario/" + std::to_string(user_id), "GET");
  std::vector<Ticket> tickets;
  for (const auto &t : response) {
    tickets.push_back({t["ID"].i(), t["Titulo"].s(), t["Descricao"].s(),
                       static_cast<int>(t["Prioridade"].i()),
                       t["ID_pessoa"].i(), static_cast<int>(t["Status"].i())});
  }
  return tickets;
}

std::vector<TicketDataManager::Pessoa> TicketDataManager::getAllUsers() {
  auto [status, response] = pImpl->makeRequest("/usuarios", "GET");
  std::vector<Pessoa> users;
  for (const auto &u : response) {
    users.push_back({u["ID"].i(), u["Login"].s(), "", (u["ADM"].b())});
  }
  return users;
}

std::optional<TicketDataManager::Pessoa>
TicketDataManager::getUserById(int64_t user_id) {
  try {
    auto [status, response] =
        pImpl->makeRequest("/usuarios/" + std::to_string(user_id), "GET");
    return json_to_pessoa(response);
  } catch (const std::exception &e) {
    return std::nullopt;
  }
}

std::optional<TicketDataManager::Pessoa>
TicketDataManager::getUserByLogin(const std::string &login) {
  try {
    auto [status, response] =
        pImpl->makeRequest("/usuarios/login/" + login, "GET");
    auto pessoa = json_to_pessoa(response);
    std::cout << response.key() << std::endl;
    return pessoa;
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return std::nullopt;
  }
}

TicketDataManager::Pessoa TicketDataManager::createUser(const PessoaDAO &user) {
  crow::json::wvalue data(
      {{"Login", user.login}, {"Senha", user.senha}, {"ADM", false}});
  std::cout << user.login << std::endl;
  std::cout << user.senha << std::endl;
  auto [status, response] = pImpl->makeRequest("/usuarios", "POST", data);
  // Não retorna a senha apenas, por isso o ""
  auto pessoa = this->getUserByLogin(user.login);

  return pessoa.value();
}

void TicketDataManager::setupRoutes(crow::SimpleApp &app) {
  CROW_ROUTE(app, "/tickets")
      .methods("GET"_method)([this](const crow::request &req) {
        auto tickets = this->getAllTickets();
        crow::json::wvalue result;
        result["tickets"] = crow::json::wvalue::list();
        for (size_t i = 0; i < tickets.size(); ++i) {
          const auto &ticket = tickets[i];
          result["tickets"][i] =
              crow::json::wvalue({{"ID", ticket.id},
                                  {"Titulo", ticket.titulo},
                                  {"Descricao", ticket.descricao},
                                  {"Prioridade", ticket.prioridade},
                                  {"ID_pessoa", ticket.id_pessoa.value_or(0)},
                                  {"Status", ticket.status}});
        }
        return crow::response(result);
      });

  CROW_ROUTE(app, "/tickets")
      .methods("POST"_method)([this](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body)
          return crow::response(400, "Invalid JSON");
        Ticket ticket = {0,
                         body["Titulo"].s(),
                         body["Descricao"].s(),
                         static_cast<int>(body["Prioridade"].i()),
                         body["ID_pessoa"].i(),
                         static_cast<int>(body["Status"].i())};
        auto created = this->createTicket(ticket);
        crow::json::wvalue result({{"ID", created.id},
                                   {"Titulo", created.titulo},
                                   {"Descricao", created.descricao},
                                   {"Prioridade", created.prioridade},
                                   {"ID_pessoa", created.id_pessoa.value_or(0)},
                                   {"Status", created.status}});
        return crow::response(result);
      });
}