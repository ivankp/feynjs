jQuery.prototype.svg = function(tag,attr) {
  const x = $(document.createElementNS('http://www.w3.org/2000/svg',tag))
    .appendTo(this);
  if (attr) x.attr(attr);
  return x;
}
// http://www.petercollingridge.co.uk/tutorials/svg/interactive/dragging/

$(() => {
  const $svg = $('#canv').svg('svg',{
    viewBox: '0 0 500 500', width: 500, height: 500
  });
  const svg = $svg[0];

  function pos(e) {
    const cmt = svg.getScreenCTM();
    return [ (e.clientX-cmt.e)/cmt.a, (e.clientY-cmt.f)/cmt.d ];
  }

  function transform(el,fs) {
    const ts = el[0].transform.baseVal;
    for (const f of fs) {
      const tr = svg.createSVGTransform();
      f(tr);
      ts.appendItem(tr);
    }
    return el;
  }

  let p0, el, tr, bnd;

  $svg.on('mousedown',function(e){
    e.preventDefault();
    if (e.target !== svg) {
      p0 = pos(e);
      el = e.target;
      tr = el.transform.baseVal.getItem(0);
      p0[0] -= tr.matrix.e;
      p0[1] -= tr.matrix.f;
      const b = el.getBBox(), v = svg.viewBox.baseVal;
      bnd = [
        v.x - b.x,
        v.width - b.x - b.width,
        v.y - b.y,
        v.height - b.y - b.height
      ];
    }
  }).on('mousemove',function(e){
    if (el) {
      const p = pos(e);
      let dx = p[0]-p0[0], dy = p[1]-p0[1];
      if (dx < bnd[0]) dx = bnd[0];
      else if (dx > bnd[1]) dx = bnd[1];
      if (dy < bnd[2]) dy = bnd[2];
      else if (dy > bnd[3]) dy = bnd[3];
      tr.setTranslate(dx,dy);
    }
  }).on('mouseup',function(e){
    el = null;
  }).on('mouseleave',function(e){
    el = null;
  });

  // https://developer.mozilla.org/en-US/docs/Web/API/SVGTransformList

  $('#tools').append([
    $('<button>').text('fermion').on('click',function(){
      transform(
        $svg.svg('path',{
          'd': 'M 0,0 100,0'
        }).css({
          'stroke': '#000000',
          'stroke-width': 4,
          'stroke-linecap': 'round',
          'stroke-linejoin': 'round'
        }),
        [ t => t.setTranslate(20,20) ]
      );
    }),
    $('<button>').text('arrow').on('click',function(){
      transform(
        $svg.svg('path',{
          'd': 'm 0,0 -5,2 1,-2 -1,-2 5,2 z'
        }).css({
          'stroke': '#000000',
          'fill': '#000000',
          'stroke-width': 1,
          'stroke-linecap': 'round',
          'stroke-linejoin': 'round'
        }),
        [ t => t.setTranslate(20,20), t => t.setScale(2.5,2.5) ]
      );
    })
  ]);
});
