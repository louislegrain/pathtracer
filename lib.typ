
#let template(body) = {
  set page(
    header: [
      #context {document.title; h(1fr); document.author.at(0)}
      #v(.5em, weak: true)
      #line(length: 100%, stroke: .5pt)
    ],
    footer: [
      #align(center)[Page #context counter(page).display() of #context counter(page).final().first()]
    ],
  )
  set image(height: 5.4cm)
  set stack(dir: ltr, spacing: 1cm)
  set table(align: center, stroke: .5pt, inset: 7pt)
  show table.cell.where(y: 0): strong

  body
}
