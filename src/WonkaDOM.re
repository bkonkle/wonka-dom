open Webapi;
open Wonka;
open Wonka.Types;
open Wonka_helpers;

/***
 * Operators
 */

/**
 * Only emit if the KeyboardEvent key matches the given key.
 */
[@genType]
let isKey: string => operatorT('a, 'a) =
  key =>
    curry(source =>
      curry(sink => {
        let talkback = ref(talkbackPlaceholder);

        source((. signal) =>
          switch (signal) {
          | Start(tb) =>
            talkback := tb;
            sink(. signal);
          | Push(event) when Dom.KeyboardEvent.key(event) !== key =>
            talkback^(. Pull)
          | _ => sink(. signal)
          }
        );
      })
    );

/**
 * Only emit if the MouseEvent button matches the given button.
 */
[@genType]
let isButton: int => operatorT('a, 'a) =
  button =>
    curry(source =>
      curry(sink => {
        let talkback = ref(talkbackPlaceholder);

        source((. signal) =>
          switch (signal) {
          | Start(tb) =>
            talkback := tb;
            sink(. signal);
          | Push(event) when Dom.MouseEvent.button(event) !== button =>
            talkback^(. Pull)
          | _ => sink(. signal)
          }
        );
      })
    );

/**
 * Temporary - will be replaced with the results of this ticket:
 * https://github.com/kitten/wonka/issues/63
 */
[@genType]
let distinct: operatorT('a, 'a) =
  curry(source =>
    curry(sink => {
      let prev = ref(None);
      let talkback = ref(talkbackPlaceholder);

      source((. signal) =>
        switch (signal) {
        | Start(tb) =>
          talkback := tb;
          sink(. signal);
        | Push(event) =>
          switch (prev^) {
          | Some(prevEvent) when event === prevEvent => talkback^(. Pull)
          | _ =>
            prev := Some(event);
            sink(. signal);
          }
        | _ => sink(. signal)
        }
      );
    })
  );

/***
 * Sources
 */

[@genType];
type mouseButton = [ | `Left | `Middle | `IE8Middle | `Right];

/**
 * Attach an event handler for a top-level window event.
 */
[@genType]
let fromWindowEvent: string => sourceT('a) =
  event =>
    curry(sink => {
      let addEventListener: (string, 'a => unit) => unit = [%raw
        {|
        function (event, handler) {
          window.addEventListener(event, handler);
        }
      |}
      ];

      let removeEventListener: (string, 'a => unit) => unit = [%raw
        {|
        function (event, handler) {
          window.removeEventListener(event, handler);
        }
      |}
      ];

      let handler = event => sink(. Push(event));

      sink(.
        Start(
          (. signal) =>
            switch (signal) {
            | Close => removeEventListener(event, handler)
            | _ => ()
            },
        ),
      );

      addEventListener(event, handler);
    });

/**
 * Creates a source which will be `true` when the key matching the given key code is pressed, and
 * `false` when it's released.
 */
[@genType]
let fromKeyPressed: string => sourceT(bool) =
  key =>
    merge([|
      fromList([false]), /* Initial value */
      fromWindowEvent("keydown") |> isKey(key) |> map((. _) => true),
      fromWindowEvent("keyup") |> isKey(key) |> map((. _) => false),
    |])
    |> distinct;

/**
 * Creates a source which will be `true` when the given mouse button is pressed, and `false` when
 * it's released.
 */
[@genType]
let fromMouseButton: int => sourceT(bool) =
  button =>
    merge([|
      fromList([false]), /* Initial valu*/
      fromWindowEvent("mousedown") |> isButton(button) |> map((. _) => true),
      fromWindowEvent("mouseup") |> isButton(button) |> map((. _) => false),
    |])
    |> distinct;

/**
 * Creates a source which will be `true` when the given mouse button is pressed, and `false` when
 * it's released. Note: in IE8 and earlier you need to use MouseIE8MiddleButton if you want to query
 * the middle button.
 */
[@genType]
let fromMouseButtonPressed: mouseButton => sourceT(bool) =
  button =>
    switch (button) {
    | `Left => fromMouseButton(0)
    | `Right => fromMouseButton(2)
    | `Middle => fromMouseButton(2)
    | `IE8Middle => fromMouseButton(4)
    };

/**
 * A source containing the current state of the touch device.
 */
[@genType]
let fromTouch: sourceT(Dom.TouchEvent.touchList) =
  merge([|
    fromWindowEvent("touchstart"),
    fromWindowEvent("touchend"),
    fromWindowEvent("touchmove"),
    fromWindowEvent("touchcancel"),
  |])
  |> map((. event) => Dom.TouchEvent.touches(event));

[@bs.get] external touchLength: Dom.TouchEvent.touchList => int = "length";

/**
 * A source which will be `true` when at least one finger is touching the touch device, and `false`
 * otherwise.
 */
let fromTap: sourceT(bool) =
  fromTouch |> map((. touches) => touchLength(touches) > 0) |> distinct;

type mousePosition = {
  x: float,
  y: float,
};

/**
 * A source containing the current mouse position.
 */
[@genType]
let fromMousePos: sourceT(mousePosition) =
  fromWindowEvent("mousemove")
  |> map((. event) => {
       let handler:
         Dom.MouseEvent.t =>
         {
           .
           "x": float,
           "y": float,
         } = [%raw
         {|
          function (e) {
            if (e.pageX && e.pageY) {
              return {x: e.pageX, y: e.pageY};
            } else if (e.clientX && e.clientY) {
              return {
                x: e.clientX + document.body.scrollLeft + document.documentElement.scrollLeft,
                y: e.clientY + document.body.scrollTop + document.documentElement.scrollTop
              };
            } else {
              throw new Error('Mouse event has no recognizable coordinates');
            }
          }
        |}
       ];

       let result = handler(event);

       {x: result##x, y: result##y};
     });

/**
 * A source which yields the current time, as determined by `now`, on every animation frame.
 */
[@genType]
let fromAnimationFrame: sourceT(float) =
  curry(sink => {
    let canceled = ref(false);
    let rec handler = t => {
      sink(. Push(t));

      if (! canceled^) {
        requestAnimationFrame(handler);
      };
    };

    requestAnimationFrame(handler);

    sink(.
      Start(
        (. signal) =>
          switch (signal) {
          | Close => canceled := true
          | _ => ()
          },
      ),
    );
  });

type windowDimensions = {
  w: int,
  h: int,
};

let getWindowDimensions = () => {
  w: Dom.window |> Dom.Window.innerWidth,
  h: Dom.window |> Dom.Window.innerHeight,
};

/**
 * A source which contains the document window's current width and height.
 */
[@genType]
let fromWindowDimensions: sourceT(windowDimensions) =
  curry(sink => {
    sink(. Push(getWindowDimensions()));

    let addEventListener: ('a => unit) => unit = [%raw
      {|
        function (handler) {
          window.addEventListener('resize', handler);
        }
      |}
    ];

    let removeEventListener: ('a => unit) => unit = [%raw
      {|
        function (handler) {
          window.removeEventListener('resize', handler);
        }
      |}
    ];

    let handler = _ => sink(. Push(getWindowDimensions()));

    sink(.
      Start(
        (. signal) =>
          switch (signal) {
          | Close => removeEventListener(handler)
          | _ => ()
          },
      ),
    );

    addEventListener(handler);
  });
