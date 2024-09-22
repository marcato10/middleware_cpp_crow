#ifndef TICKET_DATA_MANAGER_HPP
#define TICKET_DATA_MANAGER_HPP

#include "crow_all.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class TicketDataManager {
public:
  TicketDataManager(const std::string &base_url);
  ~TicketDataManager();

  TicketDataManager(const TicketDataManager &) = delete;
  TicketDataManager &operator=(const TicketDataManager &) = delete;

  TicketDataManager(TicketDataManager &&) noexcept;
  TicketDataManager &operator=(TicketDataManager &&) noexcept;

  enum class Status : int { Pending = 0, InProgress = 1, Resolved = 2 };

  struct Pessoa {
    int64_t id;
    std::string login;
    std::string senha;
    bool adm;
  };

  struct PessoaDAO {
    std::string login;
    std::string senha;
  };

  struct Ticket {
    int64_t id;
    std::string titulo;
    std::string descricao;
    int prioridade;
    std::optional<int64_t> id_pessoa;
    int status;
  };

  // Ticket operations
  std::vector<Ticket> getAllTickets();
  Ticket createTicket(const Ticket &ticket);
  Ticket updateTicket(int64_t ticket_id, const Ticket &ticket);
  std::vector<Ticket> getCompletedTickets();
  std::vector<Ticket> getIncompleteTickets();
  std::vector<Ticket> getTicketsByUser(int64_t user_id);

  // User operations
  std::vector<Pessoa> getAllUsers();
  std::optional<Pessoa> getUserById(int64_t user_id);
  std::optional<Pessoa> getUserByLogin(const std::string &login);
  Pessoa createUser(const PessoaDAO &user);

  // Setup Crow routes
  void setupRoutes(crow::SimpleApp &app);

private:
  class Impl;
  std::unique_ptr<Impl> pImpl;
};

#endif