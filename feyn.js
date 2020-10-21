// http://www.petercollingridge.co.uk/tutorials/svg/interactive/dragging/
// https://developer.mozilla.org/en-US/docs/Web/API/SVGTransformList

function _id(id) { return document.getElementById(id); }
function _el(tag) { return document.createElement(tag); }

class SVG {
  constructor(tag,p) {
    this._ = document.createElementNS('http://www.w3.org/2000/svg',tag);
    if (p) p.appendChild(this._);
  }
  attr(arg,v) {
    if (arg instanceof Object)
      for (const [key,val] of Object.entries(arg))
        this._.setAttributeNS(null,key,val);
    else if (v) this._.setAttributeNS(null,arg,v);
    else return this._.getAttributeNS(null,arg);
    return this;
  }
  style(arg,v) {
    if (arg instanceof Object)
      for (const [key,val] of Object.entries(arg))
        this._.style[key] = val;
    else if (v) this._.style[arg] = v;
    else return this._.style[arg];
    return this;
  }
}
for (const [name,type,f] of [
  ['translate', SVGTransform.SVG_TRANSFORM_TRANSLATE, (xf,x,y) => {
    xf.setTranslate(x||0,y||0);
  }],
  ['scale', SVGTransform.SVG_TRANSFORM_SCALE, (xf,x,y) => {
    xf.setScale(x||0,y||x||0);
  }]
]) {
  SVG.prototype[name] = function() {
    const xfs = this._.transform.baseVal;
    const n = xfs.numberOfItems;
    let xf;
    for (let i=0; i<n; ++i) {
      xf = xfs.getItem(i);
      if (xf.type === type) break;
      xf = null;
    }
    if (xf) {
      f(xf,...arguments);
    } else {
      xf = this._.ownerSVGElement.createSVGTransform();
      f(xf,...arguments);
      xfs.appendItem(xf);
    }
    return arguments.length ? this : xf;
  }
}

document.addEventListener('DOMContentLoaded', () => {
  const svg = new SVG('svg',_id('canv')).attr({
    viewBox: '0 0 600 600', width: 600, height: 600,
    'shape-rendering': 'geometricPrecision'
  }).style({
    'stroke': '#000000',
    'fill': 'none',
    'stroke-linecap': 'round',
    'stroke-linejoin': 'round'
  })._;

  function pos(e) {
    const cmt = svg.getScreenCTM();
    return [ (e.clientX-cmt.e)/cmt.a, (e.clientY-cmt.f)/cmt.d ];
  }

  let p0, el, tr, bnd;

  svg.addEventListener('mousedown',function(e){
    e.preventDefault();
    el = null;
    for (const x of e.composedPath()) {
      if (x!==svg) el = x;
      else break;
    }
    if (el) {
      p0 = pos(e);
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
  });
  svg.addEventListener('mousemove',function(e){
    if (el) {
      const p = pos(e);
      let x = p[0]-p0[0], y = p[1]-p0[1];
      if (x < bnd[0]) x = bnd[0];
      else if (x > bnd[1]) x = bnd[1];
      if (y < bnd[2]) y = bnd[2];
      else if (y > bnd[3]) y = bnd[3];
      tr.setTranslate(x,y);
    }
  });
  svg.addEventListener('mouseup',function(e){
    el = null;
  });
  svg.addEventListener('mouseleave',function(e){
    el = null;
  });

  const tools = new DocumentFragment();

  let btn = _el('button');
  btn.textContent = 'fermion';
  btn.addEventListener('click',function(){
    const g = new SVG('g',svg);
    const path = new SVG('path',g._).attr({
      'd': 'm 0,0 100,0'
    }).style({
      'stroke-width': 3,
    });
    g.translate(20,20);
  });
  tools.appendChild(btn);

  btn = _el('button');
  btn.textContent = 'arrow';
  btn.addEventListener('click',function(){
    const g = new SVG('g',svg);
    const path = new SVG('path',g._).attr({
      'd': 'm 0,0 -5,2 1,-2 -1,-2 5,2 z'
    }).style({
      'fill': '#000000',
      'stroke-width': 1,
    });
    path.scale(2.25);
    g.translate(20,20);
  });
  tools.appendChild(btn);

  btn = _el('button');
  btn.textContent = 'photon';
  btn.addEventListener('click',function(){
    const g = new SVG('g',svg);
    const path = new SVG('path',g._).attr({
      'd': 'm 0,0 c 4.4563,8.6668 6.0438,8.6668 10.5,0 4.4563,-8.6668 6.0438,-8.6668 10.5,0 4.4563,8.6668 6.0438,8.6668 10.5,0 4.4563,-8.6668 6.0438,-8.6668 10.5,0 4.4563,8.6668 6.0441,8.6668 10.5001,0 4.456,-8.6668 6.044,-8.6668 10.5,0 4.456,8.6668 6.044,8.6668 10.5,0 4.456,-8.6668 6.044,-8.6668 10.5,0'
      // 'd': 'M0 50 C 40 10, 60 10, 100 50 C 140 90, 160 90, 200 50 C 240 10, 260 10, 300 50 C 340 90, 360 90, 400 50'
    }).style({
      'stroke-width': 2,
    });
    path.scale(1.5);
    g.translate(20,20);
  });
  tools.appendChild(btn);

  const dummy_a = _el('a');
  btn = _el('button');
  btn.classList.add('right');
  btn.textContent = 'save';
  btn.addEventListener('click',function(){
    dummy_a.href = URL.createObjectURL(new Blob(
      [ '<?xml version="1.0" encoding="UTF-8" ?>\n',
        svg.outerHTML.replace(/^<svg/,'$& xmlns="'+svg.namespaceURI+'"') ],
      { type:"image/svg+xml;charset=utf-8" }
    ));
    dummy_a.download = 'feyn.svg';
    dummy_a.click();
  });
  tools.appendChild(btn);

  const tools_el = _id('tools');
  tools_el.appendChild(tools);
  tools_el.style.width = _id('canv').offsetWidth+'px'
});
