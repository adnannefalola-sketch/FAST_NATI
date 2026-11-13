/
//  Agence de voyage de Cotonou - Gestion des vols et des reservations
//  Application console C++ (POO) - norme C++17
/
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

//
//  Exceptions et utilitaires
//

// Erreur liee a une regle metier (operation interdite)
class ErreurMetier : public runtime_error {
public:
    explicit ErreurMetier(const string& message) : runtime_error(message) {}
};

// Levee quand le flux d'entree est ferme (Ctrl+D / fin de fichier)
class EntreeTerminee : public runtime_error {
public:
    EntreeTerminee() : runtime_error("Fin de l'entree standard") {}
};

namespace util {

string trim(const string& s) {
    size_t debut = s.find_first_not_of(" \t\r\n");
    if (debut == string::npos) return "";
    size_t fin = s.find_last_not_of(" \t\r\n");
    return s.substr(debut, fin - debut + 1);
}

string enMinuscules(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return s;
}

string enMajuscules(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return static_cast<char>(toupper(c)); });
    return s;
}

// Complete avec des espaces a droite en comptant les caracteres UTF-8 (et non les octets)
string remplir(const string& s, size_t largeur) {
    size_t caracteres = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) ++caracteres;
    return caracteres >= largeur ? s : s + string(largeur - caracteres, ' ');
}

bool contient(const string& texte, const string& motif) {
    return enMinuscules(texte).find(enMinuscules(motif)) != string::npos;
}

bool estNumerique(const string& s) {
    return !s.empty() && all_of(s.begin(), s.end(),
                                [](unsigned char c) { return isdigit(c); });
}

bool dateValide(const string& d) {  // format JJ/MM/AAAA
    if (d.size() != 10 || d[2] != '/' || d[5] != '/') return false;
    string j = d.substr(0, 2), m = d.substr(3, 2), a = d.substr(6, 4);
    if (!estNumerique(j) || !estNumerique(m) || !estNumerique(a)) return false;
    int jour = stoi(j), mois = stoi(m), annee = stoi(a);
    if (annee < 2024 || annee > 2100 || mois < 1 || mois > 12 || jour < 1) return false;
    int joursParMois[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool bissextile = (annee % 4 == 0 && annee % 100 != 0) || annee % 400 == 0;
    int maxJours = joursParMois[mois - 1] + ((mois == 2 && bissextile) ? 1 : 0);
    return jour <= maxJours;
}

bool heureValide(const string& h) {  // format HH:MM
    if (h.size() != 5 || h[2] != ':') return false;
    string hh = h.substr(0, 2), mm = h.substr(3, 2);
    if (!estNumerique(hh) || !estNumerique(mm)) return false;
    return stoi(hh) <= 23 && stoi(mm) <= 59;
}

}  // namespace util

//
//  Enumerations
//
enum class StatutReservation { EN_ATTENTE, VALIDEE, ANNULEE };
enum class StatutRemboursement { AUCUN, A_REMBOURSER, REMBOURSE };

string versTexte(StatutReservation s) {
    switch (s) {
        case StatutReservation::EN_ATTENTE: return "EN ATTENTE";
        case StatutReservation::VALIDEE:    return "VALIDEE";
        case StatutReservation::ANNULEE:    return "ANNULEE";
    }
    return "?";
}

string versTexte(StatutRemboursement s) {
    switch (s) {
        case StatutRemboursement::AUCUN:        return "Aucun";
        case StatutRemboursement::A_REMBOURSER: return "A rembourser";
        case StatutRemboursement::REMBOURSE:    return "Rembourse";
    }
    return "?";
}

//
//  Classe Passager
//
class Passager {
private:
    int id;
    string nom;
    string prenom;
    string telephone;
    string email;

public:
    Passager(int id, const string& nom, const string& prenom,
             const string& telephone, const string& email)
        : id(id), nom(nom), prenom(prenom), telephone(telephone), email(email) {}

    int getId() const { return id; }
    const string& getNom() const { return nom; }
    const string& getPrenom() const { return prenom; }
    const string& getTelephone() const { return telephone; }
    const string& getEmail() const { return email; }

    string nomComplet() const { return prenom + " " + nom; }

    void afficher() const {
        cout << "  Passager #" << id << " : " << nomComplet()
             << " | Tel : " << telephone << " | Email : " << email << "\n";
    }
};

//
//  Classe Vol
//
class Vol {
private:
    string numero;
    string depart;
    string destination;
    string date;
    string heure;
    int capacite;
    double prix;
    int placesReservees;  // reservations non annulees (en attente + validees)

public:
    Vol(const string& numero, const string& depart, const string& destination,
        const string& date, const string& heure, int capacite, double prix)
        : numero(numero), depart(depart), destination(destination), date(date),
          heure(heure), capacite(capacite), prix(prix), placesReservees(0) {}

    const string& getNumero() const { return numero; }
    const string& getDepart() const { return depart; }
    const string& getDestination() const { return destination; }
    const string& getDate() const { return date; }
    const string& getHeure() const { return heure; }
    int getCapacite() const { return capacite; }
    double getPrix() const { return prix; }
    int getPlacesReservees() const { return placesReservees; }

    int placesDisponibles() const { return capacite - placesReservees; }
    bool estComplet() const { return placesReservees >= capacite; }

    void occuperPlace() {
        if (estComplet())
            throw ErreurMetier("Le vol " + numero + " est complet (capacite : " +
                               to_string(capacite) + ").");
        ++placesReservees;
    }

    void libererPlace() {
        if (placesReservees > 0) --placesReservees;
    }

    static void afficherEntete() {
        cout << "  " << left << setw(8) << "Numero" << setw(12) << "Depart"
             << setw(14) << "Destination" << setw(12) << "Date" << setw(7) << "Heure"
             << right << setw(8) << "Places" << setw(14) << "Prix (FCFA)" << "\n";
        cout << "  " << string(75, '-') << "\n";
    }

    void afficherLigne() const {
        cout << "  " << left << setw(8) << numero << util::remplir(depart, 12)
             << util::remplir(destination, 14) << setw(12) << date << setw(7) << heure
             << right << setw(5) << placesDisponibles() << "/" << left << setw(2)
             << capacite << right << setw(14) << fixed << setprecision(0) << prix << "\n";
    }
};

//
//  Classe Reservation (associee a UN passager et UN vol)
//
class Reservation {
private:
    int numero;
    Passager* passager;
    Vol* vol;
    StatutReservation statut;
    bool paye;
    double montantPaye;
    StatutRemboursement remboursement;

public:
    Reservation(int numero, Passager* passager, Vol* vol)
        : numero(numero), passager(passager), vol(vol),
          statut(StatutReservation::EN_ATTENTE), paye(false), montantPaye(0.0),
          remboursement(StatutRemboursement::AUCUN) {
        if (passager == nullptr || vol == nullptr)
            throw ErreurMetier("Une reservation exige un passager et un vol.");
    }

    int getNumero() const { return numero; }
    Passager* getPassager() const { return passager; }
    Vol* getVol() const { return vol; }
    StatutReservation getStatut() const { return statut; }
    bool estPayee() const { return paye; }
    double getMontantPaye() const { return montantPaye; }
    StatutRemboursement getRemboursement() const { return remboursement; }
    bool estActive() const { return statut != StatutReservation::ANNULEE; }

    // Validation : la place devient definitivement reservee
    void valider() {
        if (statut != StatutReservation::EN_ATTENTE)
            throw ErreurMetier("Seule une reservation en attente peut etre validee.");
        statut = StatutReservation::VALIDEE;
    }

    // Paiement integral : valide automatiquement la reservation
    void enregistrerPaiement(double montant) {
        if (statut == StatutReservation::ANNULEE)
            throw ErreurMetier("Reservation annulee : aucun paiement possible.");
        if (paye)
            throw ErreurMetier("Cette reservation est deja payee.");
        if (montant != vol->getPrix())
            throw ErreurMetier("Montant incorrect : le prix du billet est de " +
                               to_string(static_cast<long long>(vol->getPrix())) +
                               " FCFA (paiement integral exige).");
        paye = true;
        montantPaye = montant;
        valider();
    }

    // Annulation : si la reservation etait payee, un remboursement est du
    void annuler() {
        if (statut == StatutReservation::ANNULEE)
            throw ErreurMetier("Cette reservation est deja annulee.");
        if (paye) remboursement = StatutRemboursement::A_REMBOURSER;
        statut = StatutReservation::ANNULEE;
    }

    void marquerRembourse() {
        if (remboursement != StatutRemboursement::A_REMBOURSER)
            throw ErreurMetier("Aucun remboursement en attente pour cette reservation.");
        remboursement = StatutRemboursement::REMBOURSE;
    }

    void afficher() const {
        cout << "  ----------------------------------------------\n"
             << "  Reservation n° " << numero << "\n"
             << "  Passager       : " << passager->nomComplet() << " (#" << passager->getId() << ")\n"
             << "  Vol            : " << vol->getNumero() << " (" << vol->getDepart() << " -> "
             << vol->getDestination() << ")\n"
             << "  Date / Heure   : " << vol->getDate() << " a " << vol->getHeure() << "\n"
             << "  Prix du billet : " << fixed << setprecision(0) << vol->getPrix() << " FCFA\n"
             << "  Statut         : " << versTexte(statut) << "\n"
             << "  Paiement       : " << (paye ? "Paye (" + to_string(static_cast<long long>(montantPaye)) + " FCFA)" : string("Non paye")) << "\n"
             << "  Remboursement  : " << versTexte(remboursement) << "\n"
             << "  Place          : "
             << (statut == StatutReservation::VALIDEE ? "definitivement reservee"
                 : statut == StatutReservation::EN_ATTENTE ? "retenue provisoirement"
                                                           : "liberee")
             << "\n";
    }
};

//
//  Classe Agence : possede les collections (vector) et applique les regles
//
class Agence {
private:
    string nom;
    vector<unique_ptr<Passager>> passagers;
    vector<unique_ptr<Vol>> vols;
    vector<unique_ptr<Reservation>> reservations;
    int prochainIdPassager;
    int prochainNumeroReservation;

public:
    explicit Agence(const string& nom)
        : nom(nom), prochainIdPassager(1), prochainNumeroReservation(1001) {}

    const string& getNom() const { return nom; }

    // --- Recherches ---------------------------------------------------------
    Passager* trouverPassager(int id) const {
        for (const auto& p : passagers)
            if (p->getId() == id) return p.get();
        return nullptr;
    }

    Vol* trouverVol(const string& numero) const {
        for (const auto& v : vols)
            if (v->getNumero() == util::enMajuscules(numero)) return v.get();
        return nullptr;
    }

    Reservation* trouverReservation(int numero) const {
        for (const auto& r : reservations)
            if (r->getNumero() == numero) return r.get();
        return nullptr;
    }

    // --- Passagers ----------------------------------------------------------
    Passager* enregistrerPassager(const string& nomP, const string& prenom,
                                  const string& telephone, const string& email) {
        for (const auto& p : passagers)
            if (p->getTelephone() == telephone)
                throw ErreurMetier("Un passager avec ce telephone existe deja (#" +
                                   to_string(p->getId()) + ").");
        passagers.push_back(make_unique<Passager>(prochainIdPassager++, nomP, prenom,
                                                  telephone, email));
        return passagers.back().get();
    }

    // --- Vols ---------------------------------------------------------------
    Vol* ajouterVol(const string& numero, const string& depart, const string& destination,
                    const string& date, const string& heure, int capacite, double prix) {
        if (trouverVol(numero) != nullptr)
            throw ErreurMetier("Le numero de vol " + util::enMajuscules(numero) + " existe deja.");
        vols.push_back(make_unique<Vol>(util::enMajuscules(numero), depart, destination,
                                        date, heure, capacite, prix));
        return vols.back().get();
    }

    void afficherVolsDisponibles() const {
        bool trouve = false;
        for (const auto& v : vols) {
            if (v->estComplet()) continue;
            if (!trouve) { Vol::afficherEntete(); trouve = true; }
            v->afficherLigne();
        }
        if (!trouve) cout << "  Aucun vol disponible pour le moment.\n";
    }

    void rechercherVol(const string& critere) const {
        bool trouve = false;
        for (const auto& v : vols) {
            if (util::contient(v->getNumero(), critere) ||
                util::contient(v->getDepart(), critere) ||
                util::contient(v->getDestination(), critere) ||
                util::contient(v->getDate(), critere)) {
                if (!trouve) { Vol::afficherEntete(); trouve = true; }
                v->afficherLigne();
            }
        }
        if (!trouve) cout << "  Aucun vol ne correspond a \"" << critere << "\".\n";
    }

    // --- Reservations -------------------------------------------------------
    Reservation* effectuerReservation(int idPassager, const string& numeroVol) {
        Passager* p = trouverPassager(idPassager);
        if (p == nullptr)
            throw ErreurMetier("Passager #" + to_string(idPassager) +
                               " introuvable : il doit etre enregistre avant de reserver.");
        Vol* v = trouverVol(numeroVol);
        if (v == nullptr)
            throw ErreurMetier("Vol " + util::enMajuscules(numeroVol) + " introuvable.");

        for (const auto& r : reservations)
            if (r->estActive() && r->getPassager() == p && r->getVol() == v)
                throw ErreurMetier("Ce passager a deja reserve le vol " + v->getNumero() +
                                   " (reservation n° " + to_string(r->getNumero()) + ").");

        v->occuperPlace();  // leve une erreur si le vol est complet
        reservations.push_back(make_unique<Reservation>(prochainNumeroReservation++, p, v));
        return reservations.back().get();
    }

    void enregistrerPaiement(int numeroReservation, double montant) {
        Reservation* r = trouverReservation(numeroReservation);
        if (r == nullptr)
            throw ErreurMetier("Reservation n° " + to_string(numeroReservation) + " introuvable.");
        r->enregistrerPaiement(montant);
    }

    // Retourne true si un remboursement est du
    bool annulerReservation(int numeroReservation) {
        Reservation* r = trouverReservation(numeroReservation);
        if (r == nullptr)
            throw ErreurMetier("Reservation n° " + to_string(numeroReservation) + " introuvable.");
        r->annuler();
        r->getVol()->libererPlace();
        return r->getRemboursement() == StatutRemboursement::A_REMBOURSER;
    }

    void rembourser(int numeroReservation) {
        Reservation* r = trouverReservation(numeroReservation);
        if (r == nullptr)
            throw ErreurMetier("Reservation n° " + to_string(numeroReservation) + " introuvable.");
        r->marquerRembourse();
    }

    void consulterReservations() const {
        if (reservations.empty()) {
            cout << "  Aucune reservation enregistree.\n";
            return;
        }
        cout << "  " << left << setw(8) << "Num" << setw(22) << "Passager" << setw(8) << "Vol"
             << setw(12) << "Statut" << setw(10) << "Paiement" << "Remboursement\n";
        cout << "  " << string(75, '-') << "\n";
        for (const auto& r : reservations) {
            cout << "  " << left << setw(8) << r->getNumero()
                 << util::remplir(r->getPassager()->nomComplet(), 22) << setw(8) << r->getVol()->getNumero()
                 << setw(12) << versTexte(r->getStatut()) << setw(10)
                 << (r->estPayee() ? "Paye" : "Non paye")
                 << versTexte(r->getRemboursement()) << "\n";
        }
    }

    void afficherReservation(int numero) const {
        Reservation* r = trouverReservation(numero);
        if (r == nullptr)
            throw ErreurMetier("Reservation n° " + to_string(numero) + " introuvable.");
        r->afficher();
    }
};

//
//  Classe Menu : interface console et controle des saisies
//
class Menu {
private:
    Agence& agence;

    // Reproduit la saisie a l'ecran quand l'entree vient d'un fichier/redirection
    static string lireEcho(const string& message) {
        cout << message << flush;
        string ligne;
        if (!getline(cin, ligne)) throw EntreeTerminee();
        ligne = util::trim(ligne);
        if (!isatty_stdin()) cout << ligne << "\n";
        return ligne;
    }

    static bool isatty_stdin();

    static string saisirTexte(const string& message) {
        while (true) {
            string s = lireEcho(message);
            if (!s.empty()) return s;
            cout << "  [!] Ce champ ne peut pas etre vide.\n";
        }
    }

    static string saisirNom(const string& message) {
        while (true) {
            string s = saisirTexte(message);
            bool ok = all_of(s.begin(), s.end(), [](unsigned char c) {
                return isalpha(c) || c >= 128 || c == ' ' || c == '-' || c == '\'';
            });
            if (ok) return s;
            cout << "  [!] Utilisez uniquement des lettres, espaces ou tirets.\n";
        }
    }

    static string saisirTelephone(const string& message) {
        while (true) {
            string s = saisirTexte(message);
            string chiffres;
            for (char c : s) if (c != ' ') chiffres += c;
            if (util::estNumerique(chiffres) && chiffres.size() >= 8 && chiffres.size() <= 15)
                return chiffres;
            cout << "  [!] Telephone invalide (8 a 15 chiffres attendus).\n";
        }
    }

    static string saisirEmail(const string& message) {
        while (true) {
            string s = saisirTexte(message);
            size_t at = s.find('@');
            if (at != string::npos && at > 0 && s.find('.', at) != string::npos &&
                s.find(' ') == string::npos && s.back() != '.')
                return s;
            cout << "  [!] Adresse e-mail invalide (exemple : nom@domaine.com).\n";
        }
    }

    static int saisirEntier(const string& message, int min, int max) {
        while (true) {
            string s = lireEcho(message);
            try {
                size_t pos = 0;
                int valeur = stoi(s, &pos);
                if (pos != s.size()) throw invalid_argument("reste");
                if (valeur >= min && valeur <= max) return valeur;
                cout << "  [!] Valeur hors limites (" << min << " a " << max << ").\n";
            } catch (const exception&) {
                cout << "  [!] Saisie invalide : entrez un nombre entier.\n";
            }
        }
    }

    static double saisirReel(const string& message, double min) {
        while (true) {
            string s = lireEcho(message);
            try {
                size_t pos = 0;
                double valeur = stod(s, &pos);
                if (pos != s.size()) throw invalid_argument("reste");
                if (valeur >= min) return valeur;
                cout << "  [!] La valeur doit etre superieure ou egale a " << min << ".\n";
            } catch (const exception&) {
                cout << "  [!] Saisie invalide : entrez un nombre.\n";
            }
        }
    }

    static string saisirDate(const string& message) {
        while (true) {
            string s = saisirTexte(message);
            if (util::dateValide(s)) return s;
            cout << "  [!] Date invalide (format JJ/MM/AAAA, annee >= 2024).\n";
        }
    }

    static string saisirHeure(const string& message) {
        while (true) {
            string s = saisirTexte(message);
            if (util::heureValide(s)) return s;
            cout << "  [!] Heure invalide (format HH:MM).\n";
        }
    }

    static bool saisirOuiNon(const string& message) {
        while (true) {
            string s = util::enMinuscules(lireEcho(message));
            if (s == "o" || s == "oui") return true;
            if (s == "n" || s == "non") return false;
            cout << "  [!] Repondez par o (oui) ou n (non).\n";
        }
    }

    // --- Actions du menu ----------------------------------------------------
    void actionEnregistrerPassager() {
        cout << "\n--- Enregistrer un passager ---\n";
        string nom = saisirNom("  Nom : ");
        string prenom = saisirNom("  Prenom : ");
        string tel = saisirTelephone("  Telephone : ");
        string email = saisirEmail("  Email : ");
        Passager* p = agence.enregistrerPassager(nom, prenom, tel, email);
        cout << "  [OK] Passager enregistre avec l'identifiant #" << p->getId() << ".\n";
    }

    void actionAjouterVol() {
        cout << "\n--- Ajouter un vol ---\n";
        string numero = saisirTexte("  Numero du vol (ex : AF101) : ");
        string depart = saisirNom("  Ville de depart : ");
        string destination = saisirNom("  Destination : ");
        if (util::enMinuscules(depart) == util::enMinuscules(destination)) {
            cout << "  [!] Le depart et la destination doivent etre differents.\n";
            return;
        }
        string date = saisirDate("  Date (JJ/MM/AAAA) : ");
        string heure = saisirHeure("  Heure (HH:MM) : ");
        int capacite = saisirEntier("  Capacite (1 a 500) : ", 1, 500);
        double prix = saisirReel("  Prix du billet (FCFA) : ", 1);
        Vol* v = agence.ajouterVol(numero, depart, destination, date, heure, capacite, prix);
        cout << "  [OK] Vol " << v->getNumero() << " ajoute.\n";
    }

    void actionRechercherVol() {
        cout << "\n--- Rechercher un vol ---\n";
        string critere = saisirTexte("  Numero, ville ou date : ");
        agence.rechercherVol(critere);
    }

    void actionReserver() {
        cout << "\n--- Effectuer une reservation ---\n";
        int id = saisirEntier("  Identifiant du passager : ", 1, 1000000);
        string numeroVol = saisirTexte("  Numero du vol : ");
        Reservation* r = agence.effectuerReservation(id, numeroVol);
        cout << "  [OK] Reservation n° " << r->getNumero() << " creee (EN ATTENTE).\n"
             << "       Place retenue provisoirement : payez pour la valider definitivement.\n";
    }

    void actionPayer() {
        cout << "\n--- Enregistrer un paiement ---\n";
        int num = saisirEntier("  Numero de la reservation : ", 1, 1000000);
        Reservation* r = agence.trouverReservation(num);
        if (r != nullptr && r->getStatut() != StatutReservation::ANNULEE && !r->estPayee())
            cout << "  Prix du billet : " << fixed << setprecision(0)
                 << r->getVol()->getPrix() << " FCFA\n";
        double montant = saisirReel("  Montant verse (FCFA) : ", 0);
        agence.enregistrerPaiement(num, montant);
        cout << "  [OK] Paiement enregistre. Reservation VALIDEE : place definitivement reservee.\n";
    }

    void actionAnnuler() {
        cout << "\n--- Annuler une reservation ---\n";
        int num = saisirEntier("  Numero de la reservation : ", 1, 1000000);
        bool remboursementDu = agence.annulerReservation(num);
        cout << "  [OK] Reservation n° " << num << " annulee, la place est liberee.\n";
        if (remboursementDu) {
            cout << "  Cette reservation etait payee : un remboursement est du.\n";
            if (saisirOuiNon("  Remboursement effectue maintenant ? (o/n) : ")) {
                agence.rembourser(num);
                cout << "  [OK] Remboursement enregistre.\n";
            } else {
                cout << "  Statut du remboursement : A REMBOURSER.\n";
            }
        }
    }

    void actionAfficherReservation() {
        cout << "\n--- Informations d'une reservation ---\n";
        int num = saisirEntier("  Numero de la reservation : ", 1, 1000000);
        agence.afficherReservation(num);
    }

public:
    explicit Menu(Agence& agence) : agence(agence) {}

    void afficher() const {
        cout << "\n==============================================\n"
             << "   " << agence.getNom() << "\n"
             << "==============================================\n"
             << "  1. Enregistrer un passager\n"
             << "  2. Ajouter un vol\n"
             << "  3. Afficher les vols disponibles\n"
             << "  4. Rechercher un vol\n"
             << "  5. Effectuer une reservation\n"
             << "  6. Enregistrer un paiement\n"
             << "  7. Annuler une reservation\n"
             << "  8. Consulter les reservations\n"
             << "  9. Afficher les informations d'une reservation\n"
             << "  10. Quitter\n"
             << "----------------------------------------------\n";
    }

    void executer() {
        int choix = 0;
        do {
            afficher();
            choix = saisirEntier("  Votre choix : ", 1, 10);
            try {
                switch (choix) {
                    case 1: actionEnregistrerPassager(); break;
                    case 2: actionAjouterVol(); break;
                    case 3:
                        cout << "\n--- Vols disponibles ---\n";
                        agence.afficherVolsDisponibles();
                        break;
                    case 4: actionRechercherVol(); break;
                    case 5: actionReserver(); break;
                    case 6: actionPayer(); break;
                    case 7: actionAnnuler(); break;
                    case 8:
                        cout << "\n--- Liste des reservations ---\n";
                        agence.consulterReservations();
                        break;
                    case 9: actionAfficherReservation(); break;
                    case 10: cout << "\n  Merci d'avoir utilise l'application. Au revoir !\n"; break;
                }
            } catch (const ErreurMetier& e) {
                cout << "  [ERREUR] " << e.what() << "\n";
            }
        } while (choix != 10);
    }
};

#if defined(_WIN32)
#include <io.h>
bool Menu::isatty_stdin() { return _isatty(_fileno(stdin)) != 0; }
#else
#include <unistd.h>
bool Menu::isatty_stdin() { return isatty(fileno(stdin)) != 0; }
#endif

//
//  Programme principal
//
int main() {
    Agence agence("AGENCE DE VOYAGE DE COTONOU");
    Menu menu(agence);
    try {
        menu.executer();
    } catch (const EntreeTerminee&) {
        cout << "\n  Fin de l'entree : arret du programme.\n";
    }
    return 0;
}
