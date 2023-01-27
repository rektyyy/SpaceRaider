# SpaceRaider

### Projekt „Space Raider” Michał Wilczkowski, Julian Włodarek

Zamysłem projektu jest stworzenie gry zręcznościowej opierającej się na nieskończonym unikaniu losowo generowanych asteroid statkiem kosmicznym. Aby tego dokonać, gracz musi poruszać się po ograniczonym polu na dole ekranu. Może też skorzystać z broni aby niszczyć asteroidy. Każda uniknięta asteroida zliczana jest jako punkt. Projekt tworzymy w programie Code::Blocks przy użyciu biblioteki SDL2 oraz C++. Mechanika gry Gdy gracz jest w menu używa strzałek i klawisza „Esc” do poruszania się po nim. By wybrać daną opcję musi kliknąć „Enter”.

W grze gracz może używać klawiszy strzałek by poruszać się statkiem oraz spacji by wystrzelić pocisk. Może też nacisnąć „Esc” aby zakończyć grę. Gdy pocisk trafi w asteroidę, ta ulega zniszczeniu (większe asteroidy mają 2 „życia”) – znika razem z pociskiem. W momencie próby wylotu statkiem poza dozwolony obszar gry, ten blokuje się o nią.

Gdy nastąpi kolizja asteroidy z graczem, gra się kończy. Na koniec użytkownik widzi swój wynik oraz czas przez jaki grał.
