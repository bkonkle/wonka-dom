
<br>
<a href="https://npmjs.com/package/wonka-dom">
  <img alt="NPM Version" src="https://img.shields.io/npm/v/wonka-dom.svg" />
</a>
<a href="https://npmjs.com/package/wonka-dom">
  <img alt="License" src="https://img.shields.io/npm/l/wonka-dom.svg" />
</a>
<!-- <a href="https://coveralls.io/github/bkonkle/wonka-dom?branch=master">
  <img src="https://coveralls.io/repos/github/bkonkle/wonka-dom/badge.svg?branch=master" alt="Test Coverage" />
</a> -->
<a href="https://bundlephobia.com/result?p=wonka-dom">
  <img alt="Minified gzip size" src="https://img.shields.io/bundlephobia/minzip/wonka-dom.svg?label=gzip%20size" />
</a>

# WonkaDOM

Tools for working with DOM events using the [Wonka](https://github.com/kitten/wonka) FRP library.

## Peer Dependencies

This library depends on the Wonka FRP library, and you'll want to install it directly and use it along with WonkaDOM.

- [Wonka](https://github.com/kitten/wonka) v4

## Usage

### ReasonML

Use Yarn or npm to install:

```sh
yarn add wonka wonka-dom
```

And add to your `bsconfig.json` along with `wonka`:

```json
  "bs-dependencies": ["wonka", "wonka-dom"],
```

Then `open` the `WonkaDOM` module along with `Wonka`:

```re
open Wonka;
open WonkaDOM;

fromAnimationFrame
  |> subscribe((. time) => Js.log(time));
```

### TypeScript

Use Yarn or npm to install:

```sh
yarn add wonka wonka-dom
```

Then, import functions from `wonka-dom`:

```ts
import {pipe, subscribe} from 'wonka'
import {fromAnimationFrame} from 'wonka-dom'

pipe(
  fromAnimationFrame,
  subscribe(time => {
    console.log(time)
  })
)
```

## Demo

See the Mario demo [here](https://github.com/bkonkle/wonka-mario)!

## API

### Sources

#### `fromWindowEvent: : string => sourceT('a)`

Attach an event handler for a top-level window event.

```re
fromWindowEvent("keydown")
  |> isKey("ArrowLeft")
  |> subscribe((. event) => Js.log(event));
```

#### `fromKeyPressed: string => sourceT(bool)`

Creates a source which will emit `true` when the key matching the given key code is pressed, and `false` when it's released.

```re
fromKeyPressed("ArrowLeft")
  |> subscribe((. pressed) => Js.log(pressed));
```

#### `fromMouseButton: int => sourceT(bool)`

Creates a source which will emit `true` when the given mouse button is pressed, and `false` when it's released.

```re
fromMouseButton(1)
  |> subscribe((. pressed) => Js.log(pressed));
```

#### `fromMouseButtonPressed: mouseButton => sourceT(bool)`

Creates a source which will be `true` when the given mouse button is pressed, and `false` when it's released. Note: in IE8 and earlier you need to use MouseIE8MiddleButton if you want to query the middle button.

```re
fromMouseButtonPressed(`Right)
  |> subscribe((. pressed) => Js.log(pressed));
```

#### `fromTouch: sourceT(Dom.TouchEvent.touchList)`

A source containing the current state of the touch device.

```re
fromTouch
  |> subscribe((. touches) => Js.log(touches));
```

#### `fromTap: sourceT(bool)`

A source which will be `true` when at least one finger is touching the touch device, and `false` otherwise.

```re
fromTap
  |> subscribe((. tapped) => Js.log(tapped));
```

#### `fromMousePos: sourceT(mousePosition)`

A source containing the current mouse position.

```re
fromMousePos
  |> subscribe((. {x, y}) => Js.log2(x, y));
```

#### `fromAnimationFrame: sourceT(float)`

A source which yields the current time, as determined by `now`, on every animation frame.

```re
fromAnimationFrame
  |> subscribe((. time) => Js.log(time));
```

### Operators

#### `isKey: string => operatorT('a, 'a)`

Only emit if the KeyboardEvent key matches the given key.

```re
fromWindowEvent("keydown")
  |> isKey("ArrowLeft")
  |> subscribe((. keyEvent) => Js.log(keyEvent));
```

#### `isButton: int => operatorT('a, 'a)`

Only emit if the MouseEvent button matches the given button.

```re
fromWindowEvent("mousedown")
  |> isButton(0)
  |> subscribe((. mouseEvent) => Js.log(mouseEvent));
```

## To-Do

* [ ] Unit tests
* [ ] More sources
