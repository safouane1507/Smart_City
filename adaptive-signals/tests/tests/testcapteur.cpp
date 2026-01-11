// tests/test_sensor.cpp
#include "../include/sensorr.h"
#include "../include/adaptivelogic.h"
#include <cassert>
#include <iostream>



void test_detecterVehicule() {
    Capteur c("Test");
    c.detecterVehicule();
    c.detecterVehicule();
    assert(c.obtenirNombreTotalVehicules() == 2);
    std::cout << " Test detecterVehicule PASSED" << std::endl;
}

void test_niveauTrafic() {
    Capteur c("Test");
    c.mettreAJourFile(2);
    assert(c.obtenirNiveauTrafic() == "FAIBLE");
    
    c.mettreAJourFile(8);
    assert(c.obtenirNiveauTrafic() == "MOYEN");
    
    c.mettreAJourFile(12);
    assert(c.obtenirNiveauTrafic() == "ELEVE");
    
    c.mettreAJourFile(16);
    assert(c.obtenirNiveauTrafic() == "EMBOUTEILLAGE");
    
    std::cout << " Test niveauTrafic PASSED" << std::endl;
}

void test_embouteillage() {
    Capteur c("Test");
    c.mettreAJourFile(14);
    assert(!c.estEmbouteillage());
    
    c.mettreAJourFile(15);
    assert(c.estEmbouteillage());
    
    std::cout << " Test embouteillage PASSED" << std::endl;
}

// tests/test_logic.cpp
void test_calculDureeVerte() {
    LogiqueAdaptative logique;
    Capteur c("Test");
    
    c.mettreAJourFile(2);
    int duree = logique.calculerDureeVerte(c);
    assert(duree >= 10 && duree <= 90);
    std::cout << " Test calculDureeVerte PASSED" << std::endl;
}

void test_modeUrgence() {
    LogiqueAdaptative logique;
    Capteur c("Nord");
    
    logique.activerModeUrgence("Nord");
    assert(logique.estModeUrgence());
    
    int duree = logique.calculerDureeVerte(c);
    assert(duree == 90); // DUREE_VERTE_MAXIMALE
    
    std::cout << " Test modeUrgence PASSED" << std::endl;
}

int main() {
    test_detecterVehicule();
    test_niveauTrafic();
    test_embouteillage();
    test_calculDureeVerte();
    test_modeUrgence();
    std::cout << "\n Tous les tests sont PASSÉS !" << std::endl;
    return 0;
}