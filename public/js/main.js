// main.js – wird vom Server als application/javascript ausgeliefert

// Zeigt dass JS korrekt geladen wurde
console.log('[Server Test] JS geladen via GET /js/main.js');
console.log('[Server Test] Content-Type: application/javascript ✓');

// Aktuellen Pfad loggen
console.log('[Server Test] Aktuelle Seite:', window.location.pathname);

// Kleiner visueller Test: Blinken der accent-farbigen Elemente beim Laden
document.addEventListener('DOMContentLoaded', () => {
    const accents = document.querySelectorAll('.status-dot');
    accents.forEach((dot, i) => {
        setTimeout(() => {
            dot.style.transform = 'scale(1.5)';
            setTimeout(() => dot.style.transform = 'scale(1)', 200);
        }, i * 150);
    });

    // Zeigt Ladezeit in der Konsole
    const loadTime = performance.now();
    console.log(`[Server Test] Seite geladen in ${loadTime.toFixed(1)}ms`);
});
