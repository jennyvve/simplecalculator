/* Main van een implementatie van een expressie boom. Met poolse
 * notatie kan een expressie ingelezen worden. Deze expressie kan
 * vervolgens in infix notatie geprint worden naar een CLI, of
 * geschreven worden naar een dot file. Dot is een taal waarin graven
 * omschreven kunnen worden, zie
 * https://dreampuf.github.io/GraphvizOnline.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "cli.h"

int main(int argc, char** argv) {
    cli_loop((argc == 2 && argv[1][0] == '-' && argv[1][1] == 's'));
    return 0;
}
