
Goedendag Mevr. Hagens,
Voor een project moet ik wat statistiek gebruiken en ik ben niet zeker wat de juiste methode is, en of er betere methoden zijn.

Hier is het probleem met zo min mogelijke onrelevante info:
We hebben een functie die items in een bepaalde bin/bucket plaatst. Dit doet het via karakteristieken van de item, maar het moet tegelijkertijd *uniform* zijn in de zin van dat de functie geen bias heeft om bepaalde buckets veel meer te fullen.  [Deze pagina](https://en.wikipedia.org/wiki/Hash_function#Uniformity) legt het wat beter uit.

Ik ben bezig code te schrijven dat checkt welke functies hieraan voldoen. Dus ik wil eigenlijk een measure hebben van hoeveel het verschilt van als ik gewoon een functie hebt die het in een random bucket plaatst. Of mischien anders gezecht, ik wil weten hoe groot de kans is dat dit een uniforme distributie is.


Hiervoor heb ik 2 methoden gevonden, en nu wil ik weten of deze methoden inderdaad goed zijn voor wat ik wil, welke van deze 2 beter is, en of u mischien een betere methode kent.
Ik wil dit zo graag mogelijk in 1 (of in ieder geval, zo min mogelijk) resultaat/measure, want we werken met grote $m$'s.
Hier zijn de 2 methoden die ik gevonden heb: 

# Volgens deze [wikipedia page](https://en.wikipedia.org/wiki/Hash_function#Testing_and_measurement)
When testing a hash function, the uniformity of the distribution of hash values can be evaluated by the chi-squared test. This test is a goodness-of-fit measure: it's the actual distribution of items in buckets versus the expected (or uniform) distribution of items.

$$
\frac{\sum_{j=0}^{m-1}(b_j)(b_j+1)/2}{({n \over 2m})(n+2m-1)}
$$

where $n$ is the number of items, $m$ is the number of buckets, $b_j$ is the number of items in bucket $j$.
A ratio within one confidence interval (0.95 - 1.05) is indicative that the hash function evaluated has an expected uniform distribution. 


# According to [this site:](http://staffwww.fullcoll.edu/aclifton/cs133/assignment5.html)

The Pearson $\Chi^2$ test is a method for determining the probability that a given set of observed results (in this case, the number of items in a bin) could have been generated from a given distribution. In our case, we want our distribution to be uniform, meaning that every bin should have 
$$
expected = {n \over m}
$$ 
items in it.
To do this test, you take the hashes array and compute the value of $\Chi^2$

**xi_sq** = $= \sum_{j=0}^{m} {(expected - b_j )^2\over expected}$  

We also need to know the degrees of freedom. In this case, it's the number of bins - 1 (m-1) 

**chi_sq_distr**  = with $\Chi^2$ distributie met m-1 degrees of freedom  
**Note:** dit word met een functie gedaan waarvan de documentatie [hier te vinden is](https://www.boost.org/doc/libs/1_49_0/libs/math/doc/sf_and_dist/html/math_toolkit/dist/dist_ref/dists/chi_squared_dist.html)


Then you just use the cumulative distribution function to get the probability:

**Note:** dit wordt ook met een functie gedaan. de descriptie van de functie is "The Cumulative Distribution Function is the probability that the variable takes a value less than or equal to x. It is equivalent to the integral from -infinity to x of the Probability Density Function."

Voor deze test wil ik (volgens de pagina) een resultaat krijgen dat zo dichtbij mogelijk is bij 0.


Verder heb ik nog een vraag.
Ik ben ook van plan om te kijken hoe lang het duurt om een bepaalde actie uit te voeren.
Als simplificatie, er is een functie $y(n)$ die langer duurt om een resultaat te krijgen naarmate n groter wordt.
Ik wil dan een grafiek hebben met de tijd die deze functie neemt als y as en n als x as.

Ik ben van plan om de functie 1000 keer te runnen voor een bepaalde n, en het gemiddelde hiervan te nemen. (om de grootte van systematische en random errors ten opzichte van de werkelijke waarde te verminderen, de functie is af in miliseconden). Dit wordt dan gedaan voor 50 waarden van n tussen 1000 en 10 miljoen.

Deze test wil ik dan 30 keer herhalen, en dan het gemiddelde te plotten.
Alleen ben ik niet zeker of ik het gemiddelde of de mediaan van deze 30 waarden moet plotten.
Verder ben ik ook niet zeker hoe groot de error bars moeten zijn. 
Ik zie dat mensen verschillende error bars gebruiken. 1 standaard deviatie (std), 2 std (95%) of standard error of the mean (SE). 
Ik heb al een soortgelijk ding gedaan bij een vorig project, en de docent had alleen een opmerking. ik moest uitleggen waarom ik had gekozen voor 1 std. 
Ik zou graag verduidelijking willen op welke de beste is.