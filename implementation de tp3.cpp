// ============================================================
// TP3 - Système de gestion d'une résidence universitaire
// POO + persistance CSV (séparateur ';') - un seul fichier
// Compilation : g++ -std=c++17 main.cpp -o residence
// ============================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

// ---------- Fichiers de sauvegarde ----------
const string F_ETUDIANTS    = "etudiants.csv";
const string F_BATIMENTS    = "batiments.csv";
const string F_CHAMBRES     = "chambres.csv";
const string F_DEMANDES     = "demandes.csv";
const string F_ATTRIBUTIONS = "attributions.csv";

// ---------- Utilitaires ----------
vector<string> split(const string& s, char d = ';') {
    vector<string> r;
    string x;
    stringstream ss(s);
    while (getline(ss, x, d)) r.push_back(x);
    return r;
}

string nettoyer(string s) {
    for (char& c : s) if (c == ';') c = ',';
    return s;
}

string lireTexte(const string& msg) {
    string s;
    do {
        cout << msg;
        getline(cin, s);
        s = nettoyer(s);
    } while (s.empty());
    return s;
}

int lireEntier(const string& msg) {
    while (true) {
        cout << msg;
        string s;
        getline(cin, s);
        try { return stoi(s); }
        catch (...) { cout << "  Valeur invalide, recommence.\n"; }
    }
}

// ============================================================
// Classe Etudiant
// ============================================================
class Etudiant {
    string matricule, nom, prenom, filiere;
public:
    Etudiant(string m = "", string n = "", string p = "", string f = "")
        : matricule(m), nom(n), prenom(p), filiere(f) {}
    string getMatricule() const { return matricule; }
    string getNom() const { return nom; }
    string getPrenom() const { return prenom; }
    string getFiliere() const { return filiere; }
    string toCSV() const { return matricule + ";" + nom + ";" + prenom + ";" + filiere; }
    static Etudiant fromCSV(const vector<string>& c) { return Etudiant(c[0], c[1], c[2], c[3]); }
    static size_t nbChamps() { return 4; }
};

// ============================================================
// Classe Batiment
// ============================================================
class Batiment {
    int id;
    string nom;
public:
    Batiment(int i = 0, string n = "") : id(i), nom(n) {}
    int getId() const { return id; }
    string getNom() const { return nom; }
    string toCSV() const { return to_string(id) + ";" + nom; }
    static Batiment fromCSV(const vector<string>& c) { return Batiment(stoi(c[0]), c[1]); }
    static size_t nbChamps() { return 2; }
};

// ============================================================
// Classe Chambre
// ============================================================
class Chambre {
    int id;
    string numero;
    int idBatiment;
    int capacite;
public:
    Chambre(int i = 0, string n = "", int b = 0, int c = 1)
        : id(i), numero(n), idBatiment(b), capacite(c) {}
    int getId() const { return id; }
    string getNumero() const { return numero; }
    int getIdBatiment() const { return idBatiment; }
    int getCapacite() const { return capacite; }
    string toCSV() const {
        return to_string(id) + ";" + numero + ";" + to_string(idBatiment) + ";" + to_string(capacite);
    }
    static Chambre fromCSV(const vector<string>& c) {
        return Chambre(stoi(c[0]), c[1], stoi(c[2]), stoi(c[3]));
    }
    static size_t nbChamps() { return 4; }
};

// ============================================================
// Classe DemandeHebergement (états : voir diagramme d'états)
// ============================================================
enum class Statut { EN_ATTENTE = 0, ACCEPTEE = 1, REFUSEE = 2, TERMINEE = 3 };

string statutToString(Statut s) {
    switch (s) {
        case Statut::EN_ATTENTE: return "En attente";
        case Statut::ACCEPTEE:   return "Acceptee";
        case Statut::REFUSEE:    return "Refusee";
        case Statut::TERMINEE:   return "Terminee";
    }
    return "?";
}

class DemandeHebergement {
    int id;
    string matricule;
    string dateDemande;
    Statut statut;
public:
    DemandeHebergement(int i = 0, string m = "", string d = "", Statut s = Statut::EN_ATTENTE)
        : id(i), matricule(m), dateDemande(d), statut(s) {}
    int getId() const { return id; }
    string getMatricule() const { return matricule; }
    string getDate() const { return dateDemande; }
    Statut getStatut() const { return statut; }
    void setStatut(Statut s) { statut = s; }
    string toCSV() const {
        return to_string(id) + ";" + matricule + ";" + dateDemande + ";" + to_string((int)statut);
    }
    static DemandeHebergement fromCSV(const vector<string>& c) {
        return DemandeHebergement(stoi(c[0]), c[1], c[2], (Statut)stoi(c[3]));
    }
    static size_t nbChamps() { return 4; }
};

// ============================================================
// Classe Attribution
// ============================================================
class Attribution {
    int id;
    int idDemande;
    string matricule;
    int idChambre;
    string dateEntree;
    string dateSortie;   // "-" tant que l'étudiant occupe la chambre
public:
    Attribution(int i = 0, int d = 0, string m = "", int c = 0, string de = "", string ds = "-")
        : id(i), idDemande(d), matricule(m), idChambre(c), dateEntree(de), dateSortie(ds) {}
    int getId() const { return id; }
    int getIdDemande() const { return idDemande; }
    string getMatricule() const { return matricule; }
    int getIdChambre() const { return idChambre; }
    string getDateEntree() const { return dateEntree; }
    string getDateSortie() const { return dateSortie; }
    bool estActive() const { return dateSortie == "-"; }
    void terminer(const string& date) { dateSortie = date; }
    string toCSV() const {
        return to_string(id) + ";" + to_string(idDemande) + ";" + matricule + ";" +
               to_string(idChambre) + ";" + dateEntree + ";" + dateSortie;
    }
    static Attribution fromCSV(const vector<string>& c) {
        return Attribution(stoi(c[0]), stoi(c[1]), c[2], stoi(c[3]), c[4], c[5]);
    }
    static size_t nbChamps() { return 6; }
};

// ============================================================
// Persistance générique
// ============================================================
template <class T>
void charger(const string& fichier, vector<T>& v) {
    ifstream in(fichier);
    string ligne;
    while (getline(in, ligne)) {
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
        if (ligne.empty()) continue;
        vector<string> c = split(ligne);
        if (c.size() < T::nbChamps()) continue;
        try { v.push_back(T::fromCSV(c)); } catch (...) {}
    }
}

template <class T>
void sauvegarder(const string& fichier, const vector<T>& v) {
    ofstream out(fichier);
    for (const T& x : v) out << x.toCSV() << "\n";
}

// ============================================================
// Classe Residence (contrôleur principal)
// ============================================================
class Residence {
    string nom;
    vector<Etudiant> etudiants;
    vector<Batiment> batiments;
    vector<Chambre> chambres;
    vector<DemandeHebergement> demandes;
    vector<Attribution> attributions;

    template <class T>
    int prochainId(const vector<T>& v) const {
        int m = 0;
        for (const T& x : v) if (x.getId() > m) m = x.getId();
        return m + 1;
    }

    Etudiant* trouverEtudiant(const string& mat) {
        for (auto& e : etudiants) if (e.getMatricule() == mat) return &e;
        return nullptr;
    }
    Batiment* trouverBatiment(int id) {
        for (auto& b : batiments) if (b.getId() == id) return &b;
        return nullptr;
    }
    Chambre* trouverChambre(int id) {
        for (auto& c : chambres) if (c.getId() == id) return &c;
        return nullptr;
    }
    DemandeHebergement* trouverDemande(int id) {
        for (auto& d : demandes) if (d.getId() == id) return &d;
        return nullptr;
    }
    Attribution* attributionActive(const string& mat) {
        for (auto& a : attributions) if (a.estActive() && a.getMatricule() == mat) return &a;
        return nullptr;
    }
    bool aDemandeEnAttente(const string& mat) {
        for (auto& d : demandes)
            if (d.getMatricule() == mat && d.getStatut() == Statut::EN_ATTENTE) return true;
        return false;
    }

    int placesOccupees(int idChambre) const {
        int n = 0;
        for (const auto& a : attributions)
            if (a.estActive() && a.getIdChambre() == idChambre) n++;
        return n;
    }
    int placesLibres(const Chambre& c) const { return c.getCapacite() - placesOccupees(c.getId()); }

    string nomBatiment(int id) {
        Batiment* b = trouverBatiment(id);
        return b ? b->getNom() : "?";
    }

public:
    Residence(string n = "Residence Universitaire") : nom(n) {}

    void charger() {
        ::charger(F_ETUDIANTS, etudiants);
        ::charger(F_BATIMENTS, batiments);
        ::charger(F_CHAMBRES, chambres);
        ::charger(F_DEMANDES, demandes);
        ::charger(F_ATTRIBUTIONS, attributions);
    }
    void sauvegarder() const {
        ::sauvegarder(F_ETUDIANTS, etudiants);
        ::sauvegarder(F_BATIMENTS, batiments);
        ::sauvegarder(F_CHAMBRES, chambres);
        ::sauvegarder(F_DEMANDES, demandes);
        ::sauvegarder(F_ATTRIBUTIONS, attributions);
    }

    // ----- Administration -----
    void ajouterBatiment() {
        string n = lireTexte("Nom du batiment : ");
        batiments.push_back(Batiment(prochainId(batiments), n));
        sauvegarder();
        cout << "-> Batiment ajoute.\n";
    }

    void ajouterChambre() {
        if (batiments.empty()) { cout << "Ajoute d'abord un batiment.\n"; return; }
        afficherBatiments();
        int idB = lireEntier("ID du batiment : ");
        if (!trouverBatiment(idB)) { cout << "Batiment introuvable.\n"; return; }
        string num = lireTexte("Numero de la chambre : ");
        int cap = lireEntier("Capacite (nombre de places) : ");
        if (cap <= 0) { cout << "Capacite invalide.\n"; return; }
        chambres.push_back(Chambre(prochainId(chambres), num, idB, cap));
        sauvegarder();
        cout << "-> Chambre ajoutee.\n";
    }

    void ajouterEtudiant() {
        string mat = lireTexte("Matricule : ");
        if (trouverEtudiant(mat)) { cout << "Ce matricule existe deja.\n"; return; }
        string nomE = lireTexte("Nom : ");
        string prenom = lireTexte("Prenom : ");
        string fil = lireTexte("Filiere : ");
        etudiants.push_back(Etudiant(mat, nomE, prenom, fil));
        sauvegarder();
        cout << "-> Etudiant enregistre.\n";
    }

    // ----- Demandes d'hébergement -----
    void enregistrerDemande() {
        string mat = lireTexte("Matricule de l'etudiant : ");
        if (!trouverEtudiant(mat)) {
            cout << "Etudiant inconnu. Enregistre-le d'abord (menu 1).\n";
            return;
        }
        if (attributionActive(mat)) { cout << "Cet etudiant occupe deja une chambre.\n"; return; }
        if (aDemandeEnAttente(mat)) { cout << "Une demande est deja en attente pour cet etudiant.\n"; return; }
        string date = lireTexte("Date de la demande (JJ/MM/AAAA) : ");
        demandes.push_back(DemandeHebergement(prochainId(demandes), mat, date));
        sauvegarder();
        cout << "-> Demande enregistree (statut : En attente).\n";
    }

    void afficherDemandes(bool seulementEnAttente = false) {
        cout << "\n" << left << setw(5) << "ID" << setw(12) << "Matricule" << setw(24) << "Etudiant"
             << setw(12) << "Date" << "Statut\n" << string(66, '-') << "\n";
        int n = 0;
        for (const auto& d : demandes) {
            if (seulementEnAttente && d.getStatut() != Statut::EN_ATTENTE) continue;
            Etudiant* e = trouverEtudiant(d.getMatricule());
            string nomE = e ? e->getNom() + " " + e->getPrenom() : "?";
            cout << left << setw(5) << d.getId() << setw(12) << d.getMatricule() << setw(24) << nomE
                 << setw(12) << d.getDate() << statutToString(d.getStatut()) << "\n";
            n++;
        }
        if (n == 0) cout << "(aucune demande)\n";
    }

    void refuserDemande() {
        afficherDemandes(true);
        int id = lireEntier("ID de la demande a refuser : ");
        DemandeHebergement* d = trouverDemande(id);
        if (!d || d->getStatut() != Statut::EN_ATTENTE) { cout << "Demande introuvable ou deja traitee.\n"; return; }
        d->setStatut(Statut::REFUSEE);
        sauvegarder();
        cout << "-> Demande refusee.\n";
    }

    // ----- Attribution d'une chambre -----
    void attribuerChambre() {
        afficherDemandes(true);
        int idD = lireEntier("ID de la demande a satisfaire : ");
        DemandeHebergement* d = trouverDemande(idD);
        if (!d || d->getStatut() != Statut::EN_ATTENTE) { cout << "Demande introuvable ou deja traitee.\n"; return; }
        if (attributionActive(d->getMatricule())) { cout << "Cet etudiant a deja une chambre.\n"; return; }

        afficherChambres(2); // chambres avec places libres
        int idC = lireEntier("ID de la chambre a attribuer : ");
        Chambre* c = trouverChambre(idC);
        if (!c) { cout << "Chambre introuvable.\n"; return; }
        if (placesLibres(*c) <= 0) { cout << "Chambre complete : attribution impossible.\n"; return; }

        string date = lireTexte("Date d'entree (JJ/MM/AAAA) : ");
        attributions.push_back(Attribution(prochainId(attributions), idD, d->getMatricule(), idC, date));
        d->setStatut(Statut::ACCEPTEE);
        sauvegarder();
        cout << "-> Chambre attribuee. Demande acceptee.\n";
    }

    // ----- Départ -----
    void enregistrerDepart() {
        string mat = lireTexte("Matricule de l'etudiant qui part : ");
        Attribution* a = attributionActive(mat);
        if (!a) { cout << "Aucune occupation active pour ce matricule.\n"; return; }
        string date = lireTexte("Date de depart (JJ/MM/AAAA) : ");
        a->terminer(date);
        DemandeHebergement* d = trouverDemande(a->getIdDemande());
        if (d) d->setStatut(Statut::TERMINEE);
        sauvegarder();
        cout << "-> Depart enregistre. La place est liberee.\n";
    }

    // ----- Consultations -----
    void afficherBatiments() {
        cout << "\nBatiments :\n";
        for (const auto& b : batiments) cout << "  [" << b.getId() << "] " << b.getNom() << "\n";
    }

    // filtre : 0 = toutes, 1 = occupees, 2 = libres (au moins une place), 3 = completes
    void afficherChambres(int filtre = 0) {
        cout << "\n" << left << setw(5) << "ID" << setw(16) << "Batiment" << setw(10) << "Chambre"
             << setw(10) << "Capacite" << setw(10) << "Occupees" << "Libres\n" << string(61, '-') << "\n";
        int n = 0;
        for (const auto& c : chambres) {
            int occ = placesOccupees(c.getId());
            int lib = c.getCapacite() - occ;
            if (filtre == 1 && occ == 0) continue;
            if (filtre == 2 && lib <= 0) continue;
            if (filtre == 3 && lib > 0) continue;
            cout << left << setw(5) << c.getId() << setw(16) << nomBatiment(c.getIdBatiment())
                 << setw(10) << c.getNumero() << setw(10) << c.getCapacite()
                 << setw(10) << occ << lib << "\n";
            n++;
        }
        if (n == 0) cout << "(aucune chambre)\n";
    }

    void verifierPlacesDisponibles() {
        int total = 0, libres = 0;
        for (const auto& c : chambres) { total += c.getCapacite(); libres += placesLibres(c); }
        cout << "\nPlaces totales     : " << total
             << "\nPlaces disponibles : " << libres << "\n";
        afficherChambres(2);
    }

    void afficherHistorique() {
        cout << "Matricule (laisser vide = tout l'historique) : ";
        string mat;
        getline(cin, mat);
        cout << "\n" << left << setw(5) << "ID" << setw(12) << "Matricule" << setw(22) << "Etudiant"
             << setw(10) << "Chambre" << setw(12) << "Entree" << "Sortie\n" << string(70, '-') << "\n";
        int n = 0;
        for (const auto& a : attributions) {
            if (!mat.empty() && a.getMatricule() != mat) continue;
            Etudiant* e = trouverEtudiant(a.getMatricule());
            Chambre* c = trouverChambre(a.getIdChambre());
            string nomE = e ? e->getNom() + " " + e->getPrenom() : "?";
            string ch = c ? (nomBatiment(c->getIdBatiment()) + "-" + c->getNumero()) : "?";
            cout << left << setw(5) << a.getId() << setw(12) << a.getMatricule() << setw(22) << nomE
                 << setw(10) << ch << setw(12) << a.getDateEntree()
                 << (a.estActive() ? "(en cours)" : a.getDateSortie()) << "\n";
            n++;
        }
        if (n == 0) cout << "(aucune occupation)\n";
    }

    // ----- Extension : statistiques -----
    void afficherStatistiques() {
        int total = 0, occ = 0;
        for (const auto& c : chambres) { total += c.getCapacite(); occ += placesOccupees(c.getId()); }
        cout << fixed << setprecision(1);
        cout << "\n===== STATISTIQUES : " << nom << " =====\n";
        cout << "Etudiants enregistres : " << etudiants.size() << "\n";
        cout << "Batiments / Chambres  : " << batiments.size() << " / " << chambres.size() << "\n";
        cout << "Places occupees       : " << occ << " / " << total << "\n";
        cout << "Taux d'occupation     : " << (total ? 100.0 * occ / total : 0.0) << " %\n";

        cout << "\nTaux par batiment :\n";
        for (const auto& b : batiments) {
            int t = 0, o = 0;
            for (const auto& c : chambres)
                if (c.getIdBatiment() == b.getId()) { t += c.getCapacite(); o += placesOccupees(c.getId()); }
            cout << "  " << left << setw(16) << b.getNom() << o << "/" << t << "  ("
                 << (t ? 100.0 * o / t : 0.0) << " %)\n";
        }

        int cnt[4] = {0, 0, 0, 0};
        for (const auto& d : demandes) cnt[(int)d.getStatut()]++;
        cout << "\nDemandes : " << cnt[0] << " en attente, " << cnt[1] << " acceptees, "
             << cnt[2] << " refusees, " << cnt[3] << " terminees\n";
    }
};

// ============================================================
// Menu principal
// ============================================================
void afficherMenu() {
    cout << "\n========== RESIDENCE UNIVERSITAIRE ==========\n"
         << " 1. Enregistrer un etudiant\n"
         << " 2. Ajouter un batiment\n"
         << " 3. Ajouter une chambre\n"
         << " 4. Enregistrer une demande d'hebergement\n"
         << " 5. Attribuer une chambre\n"
         << " 6. Refuser une demande\n"
         << " 7. Enregistrer un depart\n"
         << " 8. Verifier les places disponibles\n"
         << " 9. Chambres occupees\n"
         << "10. Chambres libres\n"
         << "11. Liste des demandes\n"
         << "12. Historique des occupations\n"
         << "13. Statistiques et taux d'occupation\n"
         << " 0. Quitter\n"
         << "=============================================\n";
}

int main() {
    Residence res("Residence Universitaire");
    res.charger();

    int choix;
    do {
        afficherMenu();
        choix = lireEntier("Votre choix : ");
        switch (choix) {
            case 1:  res.ajouterEtudiant(); break;
            case 2:  res.ajouterBatiment(); break;
            case 3:  res.ajouterChambre(); break;
            case 4:  res.enregistrerDemande(); break;
            case 5:  res.attribuerChambre(); break;
            case 6:  res.refuserDemande(); break;
            case 7:  res.enregistrerDepart(); break;
            case 8:  res.verifierPlacesDisponibles(); break;
            case 9:  res.afficherChambres(1); break;
            case 10: res.afficherChambres(2); break;
            case 11: res.afficherDemandes(); break;
            case 12: res.afficherHistorique(); break;
            case 13: res.afficherStatistiques(); break;
            case 0:  cout << "Donnees sauvegardees. Au revoir !\n"; break;
            default: cout << "Choix invalide.\n";
        }
    } while (choix != 0);

    res.sauvegarder();
    return 0;
}
