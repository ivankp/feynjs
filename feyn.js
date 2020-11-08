// http://www.petercollingridge.co.uk/tutorials/svg/interactive/dragging/
// https://developer.mozilla.org/en-US/docs/Web/API/SVGTransformList

const rphi = (Math.sqrt(5)-1)/2;
const gb = 4./3.;
const ga = a => 0.038818339370 + a*0.184069954091;
const save_padding = 5;

function make(tag,p) {
  const x = document.createElement(tag);
  return p ? p.appendChild(x) : x;
}
function _id(id) { return document.getElementById(id); }

function SVG(tag,p,attr,style,classes) {
  const el = document.createElementNS('http://www.w3.org/2000/svg',tag);
  if (p) p.appendChild(el);
  if (attr) {
    if (attr.constructor === Object) {
      for (const [key,val] of Object.entries(attr))
        el.setAttributeNS(null,key,val);
    } else if (attr.constructor === String) {
      el.setAttributeNS(null,'d',attr);
    }
  }
  if (style && style.constructor === Object) {
    for (const [key,val] of Object.entries(style))
      el.style[key] = val;
  }
  // if (id) el.id = id;
  if (classes) {
    if (classes.constructor === String)
      el.classList.add(classes);
    else if (classes.constructor === Array)
      el.classList.add(...classes);
  }
  return el;
}

for (const [name,type,f] of [
  ['translate', SVGTransform.SVG_TRANSFORM_TRANSLATE, (xf,x,y) => {
    xf.setTranslate(x||0,y||0);
  }],
  ['scale', SVGTransform.SVG_TRANSFORM_SCALE, (xf,x,y) => {
    xf.setScale(x||0,y||x||0);
  }],
  ['rotate', SVGTransform.SVG_TRANSFORM_ROTATE, (xf,a,cx,cy) => {
    xf.setRotate(a,cx||0,cy||0);
  }]
]) {
  window[name] = function(el,...args) {
    const xfs = el.transform.baseVal;
    const n = xfs.numberOfItems;
    let xf;
    for (let i=0; i<n; ++i) {
      xf = xfs.getItem(i);
      if (xf.type === type) break;
      xf = null;
    }
    if (xf) {
      f(xf,...args);
    } else {
      xf = el.ownerSVGElement.createSVGTransform();
      f(xf,...args);
      xfs.appendItem(xf);
    }
    return args.length ? el : xf;
  }
}

document.addEventListener('DOMContentLoaded', () => {
  const svg = SVG('svg',_id('canv'),{
    viewBox: '0 0 500 500', width: 500, height: 500
  },{
    'stroke': '#000',
    'stroke-width': 2.5,
    'fill': 'none',
    'stroke-linecap': 'round',
    'stroke-linejoin': 'round'
  });

  function pos(e) {
    const cmt = svg.getScreenCTM();
    if (e.touches) e = e.touches[0];
    return [ (e.clientX-cmt.e)/cmt.a, (e.clientY-cmt.f)/cmt.d ];
  }

  let p0, el, tr, bnd;

  ['mousedown','touchstart'].forEach((e,ei) => {
    svg.addEventListener(e,function(e){
      if (ei===0) e.preventDefault();
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
  });
  ['mousemove','touchmove'].forEach(e => {
    svg.addEventListener(e,function(e){
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
  });
  ['mouseup','mouseleave','touchend','touchcancel'].forEach(e => {
    svg.addEventListener(e,function(e){
      el = null;
    });
  });

  const tools = new DocumentFragment();

  const left = make('div',tools);
  left.classList.add('left');

  let div = make('div',left);
  let btn = make('button',div);
  btn.textContent = 'fermion';
  btn.addEventListener('click',function(){
    const g = SVG('g',svg);
    const arrow_scale = 1.75;
    const l = [80,5], s = [,1];
    const d = 'm 0,0 '+l[0]+',0';
    SVG('path',g,d);
    const arrow = SVG('path',g,{
      d: 'm 0,0 -'+l[1]+',2 q 1.5,-2 0,-4 z',
      'fill': '#000',
      'stroke-width': s[1]
    });
    SVG('path',g,d,null,'ghost');
    translate(arrow,((l[1]+s[1])*arrow_scale+l[0])/2);
    scale(arrow,arrow_scale);
    translate(g,20,20);
  });

  div = make('div',left);
  btn = make('button',div);
  btn.textContent = 'scalar';
  btn.addEventListener('click',function(){
    const g = SVG('g',svg);
    const l = 80;
    const r = 1.2; // white / black
    let b, b0;
    for (let i=3; ; ++i) {
      b0 = l/(i+r*(i-1));
      if (b0 < 7.5) break;
      b = b0;
    }
    const d = 'm 0,0 '+l+',0';
    SVG('path',g,{ d,
      'stroke-dasharray': b.toFixed(4)+' '+(r*b).toFixed(4),
    });
    SVG('path',g,d,null,'ghost');
    translate(g,20,20);
  });

  div = make('div',left);
  btn = make('button',div);
  btn.textContent = 'photon';
  const np = make('input',div);
  np.type = 'text';
  np.value = 8;
  np.size = 2;
  btn.addEventListener('click',function(){
    const g = SVG('g',svg);
    const n = np.value;
    let d = 'm 0,0 c';
    const A = 10,
          a  = ga(2),
          a1 = (a*A).toFixed(4),
          a2 = ((1-a)*A).toFixed(4),
          b  = (gb*A*0.5).toFixed(4);
    for (let i=0; i<n; ++i) {
      if (i===0) d += ` ${a1},-${b}`;
      else if (i===1) d += ' s';
      d += ` ${a2},${i%2?'':'-'}${b} ${A},0`;
    }
    SVG('path',g,d);
    SVG('path',g,d,null,'ghost');
    translate(g,20,20);
  });

  div = make('div',left);
  btn = make('button',div);
  btn.textContent = 'gluon';
  const gluon_opts = [[15],[0.7],[2]].map(x => {
    const o = make('input',div);
    o.type = 'text';
    o.value = x[0];
    o.size = 2;
    return o;
  });
  btn.addEventListener('click',function(){
    const g = SVG('g',svg);
    const n = gluon_opts[0].value;
    let c = gluon_opts[1].value;
    if (c < 0) c = 0;
    if (c > 2) c = 2;
    const r = gluon_opts[2].value;
    let d = 'm 0,0 c';
    const Ay = 5, Ax = Ay*r,
          a  = ga(c),
          a1 = (a*Ax).toFixed(4),
          a2 = ((1-a)*Ax).toFixed(4),
          a3 = ((c-1)*Ax).toFixed(4),
          a4 = ((c-a-1)*Ax).toFixed(4),
          b  = (gb*Ay).toFixed(4);
    for (let i=0; i<n; ++i) {
      if (i===0) d += ` ${a1},-${b}`;
      else if (i===1) d += ' s';
      d += i%2
        ? ` ${a4},${ b} ${a3},0`
        : ` ${a2},${-b} ${Ax},0`;
    }
    SVG('path',g,d);
    SVG('path',g,d,null,'ghost');
    translate(g,20,20);
  });

  const right = make('div',tools);
  right.classList.add('right');

  const dummy_a = make('a');
  btn = make('button',make('div',right));
  btn.id = 'save';
  btn.textContent = 'save';
  btn.addEventListener('click',function(){
    let bb = svg.getBBox();
    bb = [ bb.x, bb.y, bb.width, bb.height ].map(x => Math.round(x));
    bb[0] -= save_padding;
    bb[1] -= save_padding;
    bb[2] += save_padding*2;
    bb[3] += save_padding*2;

    const clone = svg.cloneNode(true);
    clone.querySelectorAll('.ghost').forEach(x => x.remove());

    dummy_a.href = URL.createObjectURL(new Blob(
      [ '<?xml version="1.0" encoding="UTF-8"?>\n',
        clone.outerHTML
        .replace(/^<svg\s*([^>]*)>/, (m,_1) =>
          // add xml namespace
          '<svg xmlns="'+svg.namespaceURI+'" '+ _1
          // adjust viewBox to crop
          .replace(/(viewBox=")[^"]+/, (m,_1) => _1+bb.join(' '))
          // remove width and height
          .replace(/\s*(?:width|height)="[^"]+"/g,'')
          +'>'
        )
        // self-closing tags
        .replace(/<([^ <>\t]+)([^>]*)>\s*<\/\1>/g,'<$1$2/>')
        // terse style
        .replace(/(style=")([^"]+)/g, (m,_1,_2) =>
          _1 + _2
          .replace(/\s*:\s*/g,':')
          // hex colors
          .replace(/:rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/g,
            (m,_1,_2,_3) => [_1,_2,_3].reduce( (a,x) =>
              a+Math.round(parseFloat(x)).toString(16).padStart(2,'0'), ':#')
          )
        )
        // scale with single argument
        .replace(/scale\(([+-]?\d+(?:\.\d*)?)\s*,?\s*\1\)/g,'scale($1)')
      ],
      { type:"image/svg+xml;charset=utf-8" }
    ));
    dummy_a.download = 'feyn.svg';
    dummy_a.click();
  });

  btn = make('button',make('div',right));
  btn.textContent = 'clear';
  btn.addEventListener('click',function(){
    for (let x; x = svg.firstChild; )
      svg.removeChild(x);
  });

  const tools_el = _id('tools');
  tools_el.appendChild(tools);
  tools_el.style.width = _id('canv').offsetWidth+'px';
});

window.addEventListener('keydown', function(e) { // Ctrl + s
  if ( e.ctrlKey && !(e.shiftKey || e.altKey || e.metaKey)
    && ((e.which || e.keyCode) === 83)
  ) {
    e.preventDefault();
    _id('save').click();
  }
});
