## Rendering equation:
### $ L_o = \sum_{n} f_r (x, L_n, V)\, L_i(x, L_n) \, (L_n \cdot N) $ 
$ x = \text{fragment position} $  <br>
$ V = \text{view vector} $ <br>
$ L_n = \text{n. light vector} $ <br>
$ L_i = \text{incoming light} $ <br>
$ f_r = \text{BRDF} = k_d f_d + k_s f_s$ <br>
$ k_s = Fresnel $ <br>
$ k_d = 1-k_s$ 


## Diffuse

| Modell               | Képlet         |
|----------------------|----------------|
| Lambert                     | $$ f_d = \frac{\text{albedo}}{\pi} $$ |
| Oren–Nayar                  | $$ f_d = \frac{\text{albedo}}{\pi} \left( A + B \max(0, \cos(\phi_i - \phi_r)) \sin\alpha \tan\beta \right) $$ $$ A = 1 - \frac{\sigma^2}{2(\sigma^2 + 0.33)}, \quad B = \frac{0.45 \sigma^2}{\sigma^2 + 0.09} $$ |
| Burley / Disney Diffuse     | $$ f_d = \frac{\text{baseColor}}{\pi} (1 + (F_{90}-1)(1-\cos\theta_l)^5)(1 + (F_{90}-1)(1-\cos\theta_v)^5) $$ $$ F_{90} = 0.5 + 2 \cdot \text{roughness} \cdot \cos^2\theta_d $$ |

## Specular

| Modell               | Képlet         |
|----------------------|----------------|
| Cook–Torrance               | $$ f_s = \frac{D(\mathbf{h}) F(\mathbf{v}, \mathbf{h}) G(\mathbf{l}, \mathbf{v})}{4 (n \cdot l) (n \cdot v)} $$ |
| Blinn–Phong                 | $$ f_s = \frac{n+2}{2\pi} (n \cdot h)^n $$ |
| Ward                        | $$ f_s = \frac{1}{4\pi\alpha_x \alpha_y \sqrt{(n\cdot l)(n\cdot v)}} \exp\left(-\frac{\tan^2 \theta_h}{1}\right) $$ (anizotróp, egyszerűsítve) |
| Disney                      | $$ f_\text{clearcoat} = clearcoat \frac{D_\text{GTR1}(h, \alpha) F_\text{0.04}(v,h) G_\text{Smith}(l,v,h)}{4*(n \cdot l)(n \cdot v)},  α=lerp(0.1,0.001,clearcoatGloss) $$ $$ f_\text{sheen} = sheen \cdot F_\text{sheen}(\omega_i, \omega_o)C_\text{sheen}, C_\text{sheen} = lerp(1, normalize(baseColor), sheenTint) $$ $$ f_r = f_d + f_\text{sCookTorrance} + f_\text{sheen} + f_\text{clearcoat} $$|

## NDF

| Modell               | Képlet         |
|----------------------|----------------|
| GGX / Trowbridge–Reitz      | $$ D_\text{GGX}(\mathbf{h}) = \frac{\alpha^2}{\pi ((n \cdot h)^2 (\alpha^2 - 1) + 1)^2} $$ |
| Beckmann NDF                | $$ D_\text{Beckmann}(\mathbf{h}) = \frac{1}{\pi \alpha^2 \cos^4 \theta_h} \exp\left(-\frac{\tan^2 \theta_h}{\alpha^2}\right) $$ |
| Generalized-Trowbridge-Reitz| $$ D_\text{GTR1} = \frac{c}{\alpha ^ 2 \cos^2 \theta_h + \sin ^2 \theta_h} $$ |

## Geometry

| Modell               | Képlet         |
|----------------------|----------------|
| Smith (UE4)          | $$ G_\text{Smith}​(\mathbf{l,v,\alpha})=G_1​(\mathbf{l}) G_1​(\mathbf{v}) $$ $$ G_1(\mathbf{v}) = \frac{n \cdot v}{(n \cdot v)(1-k)+k}, k=\frac{(\alpha + 1)^2}{8} $$|


## Fresnel

| Modell               | Képlet         |
|----------------------|----------------|
| Schlick Fresnel             | $$ F(\theta) = F_0 + (1 - F_0)(1 - \cos\theta)^5 $$ |

## Jelmagyarázat

| Paraméter             | Leírás     |
|-----------------------|------------|
| $$f_d$$               | Diffúz BRDF érték |
| $$f_s$$               | Specular BRDF érték |
| $$\text{albedo}$$     | Felület alapszíne (diffúz szín) |
| $$\text{baseColor}$$  | Disney BRDF-ben a diffúz szín |
| $$\theta_l$$           | Beeső fény szöge a felület normáljához képest |
| $$\theta_v$$           | Kamera / néző szöge a normálhoz képest |
| $$\theta_d$$           | Két irány közötti szög: $\theta_d = \theta_l - \theta_v$ |
| $$\theta_h$$           | Normálvektor és half-vektor közötti szög: $\theta_h = \arccos{(n \cdot h)}$ |
| $$F_{90}$$             | Fresnel-szerű faktor a diffúzhoz (Disney BRDF) |
| $$\text{roughness}$$   | Felület durvasága (0 = sima, 1 = nagyon durva) |
| $$\mathbf{n}$$         | Felület normálvektora |
| $$\mathbf{l}$$         | Bejövő fény iránya (light vector) |
| $$\mathbf{v}$$         | Néző / kamera iránya (view vector) |
| $$\mathbf{h}$$         | Half-vector: $\mathbf{h} = \frac{\mathbf{l} + \mathbf{v}}{\|\mathbf{l} + \mathbf{v}\|}$ |
| $$D(\mathbf{h})$$      | Normal Distribution Function (NDF) – mikrofelület eloszlás |
| $$F(\mathbf{v},\mathbf{h})$$| Fresnel-visszaverődés a néző irányában |
| $$G(\mathbf{l},\mathbf{v})$$| Geometry / shadowing-masking faktor |
| $$F_0$$               | Fresnel visszaverődés normál irányból (fémeknél színes, dielektrikumoknál ~0.04) |
| $$\alpha$$             | Roughness paraméter a GGX/Beckmann NDF-hez |
| $$n$$                  | Phong exponens a Blinn–Phong modellben |
| $$R_s, R_p$$           | Fresnel visszaverődés s- és p-polarizációban |
| $$R$$                  | Teljes Fresnel visszaverődés (átlag: $R = (R_s + R_p)/2$) |
| $$\sigma$$             | Felületi durvaság az Oren–Nayar modellben |
| $$A, B$$               | Oren–Nayar konstansok, $\sigma$-tól függően |
| $$\phi_i, \phi_r$$     | Bejövő és kimenő irányok az Oren–Nayar modellben |
| $$\alpha, \beta$$      | Oren–Nayar szögparaméterek: $\alpha = \max(\theta_i, \theta_r), \beta = \min(\theta_i, \theta_r)$ |
