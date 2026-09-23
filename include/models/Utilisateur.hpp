#pragma once

#include <string>
#include <utility>

enum class Role
{
    ADMIN,
    GESTIONNAIRE,
    MAGASINIER
};

class Utilisateur
{
    private:
        int id;
        std::string username;
        std::string passwordHash;
        Role role;

    public:
        Utilisateur(int idU, std::string user, std::string pwdHash, Role r)
            : id(idU), username(std::move(user)), passwordHash(std::move(pwdHash)), role(r) {}

        int getId() const { return id; }
        const std::string& getUsername() const { return username; }
        Role getRole() const { return role; }

        bool verifierMotDePasse(const std::string& pwdHashSaisi) const
        {
            return passwordHash == pwdHashSaisi;
        }
};